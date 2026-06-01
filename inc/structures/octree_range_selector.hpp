#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "../geometry/point.hpp"
#include "../main_options.hpp"
#include "octree_types.hpp"
#include "linear_octree.hpp"

inline std::filesystem::path getFindAndInsertPointsLogPath();
// ---------------------------------------------------------------

const double UMBRAL_PCT_PODA = 0.5; // Si el rango seleccionado es menor que este porcentaje del total, se devuelve ese rango sin buscar en el resto de claves

namespace {
    double accumulatedProjectionTime = 0.0;
    double accumulatedBinarySearchTime = 0.0;
}


inline void writeProfileHeaderIfNeeded(std::ofstream& logFile, const std::filesystem::path& logPath, bool& headerWritten) {
    if (headerWritten) {
        return;
    }

    headerWritten = true;
    std::error_code ec;
    // IMPORTANTE: Asegúrate de que compruebe si el archivo físico ya existe en disco 
    // creado por la otra función de log.
    if (!std::filesystem::exists(logPath, ec) || std::filesystem::file_size(logPath, ec) == 0) {
        logFile << "kernel,radius,mode,get_range_time,loop_time,projectionTime,binarySearchTime,threshold,maxPointsLeaf\n";
    }
}
inline void resetRangeSelectorAccumulators() {
    accumulatedProjectionTime = 0.0;
    accumulatedBinarySearchTime = 0.0;
}

inline void writeRangeSelectorProfileLog(const std::string& kernelName, double radius, const std::string& mode) {
    if (!mainOptions.debugLeavesTime) {
        return;
    }

    const std::filesystem::path logPath = getFindAndInsertPointsLogPath();
    static bool rangeSelectorProfileHeaderWritten = false;
    std::ofstream logFile(logPath, std::ios::app);
    if (!logFile.is_open()) {
        return;
    }

    writeProfileHeaderIfNeeded(logFile, logPath, rangeSelectorProfileHeaderWritten);

    logFile << kernelName << ","
            << std::to_string(radius) << ","
            << mode << ","
            << "0.0"
            << ","
            << "0.0"
            << ","
            << accumulatedProjectionTime << ","
            << accumulatedBinarySearchTime << ","
            << std::to_string(mainOptions.umbralPoda) << ","
            << std::to_string(mainOptions.maxPointsLeaf) << "\n";
}


// =========================================================================
// FUNCIONES DE BÚSQUEDA BINARIA SIN SALTOS (BRANCHLESS)
// =========================================================================
inline size_t branchless_lower_bound(const std::vector<double>& keys, double val) {
    size_t base = 0;
    size_t len = keys.size();
    
    while (len > 1) {
        size_t half = len >> 1;
        base += (keys[base + half - 1] < val) * half;
        len -= half;
    }
    
    if (!keys.empty() && keys[base] < val) {
        base++;
    }
    return base;
}

inline size_t branchless_upper_bound(const std::vector<double>& keys, double val) {
    size_t base = 0;
    size_t len = keys.size();
    
    while (len > 1) {
        size_t half = len >> 1;
        base += (keys[base + half - 1] <= val) * half;
        len -= half;
    }
    
    if (!keys.empty() && keys[base] <= val) {
        base++;
    }
    return base;
}

// ###########################################################################################
// ########################## MODO CARTESIANO  ###############################
// ###########################################################################################
template<typename Octree_t, typename Reordered_t>
inline PrunedRange bestRangeCartesian(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    size_t count,
    const Octree_t& octree,
    const Reordered_t& reordered,
    const Vector& leafRadii)
{
    const Point& center = octree.getLeafCenter(leaf);
    PrunedRange best{0, count, 0, 0, false, OrderType::K0};
    const double eps = 1e-9;
    // Evaluamos los 3 ejes de forma directa
    for (OrderType order : {OrderType::K0, OrderType::K1, OrderType::K2}) {
        double qCoord = 0.0;
        double hRadius = 0.0;

        if (order == OrderType::K0) {
            qCoord = query.getX() - center.getX();
            hRadius = leafRadii.getX();
        } else if (order == OrderType::K1) {
            qCoord = query.getY() - center.getY();
            hRadius = leafRadii.getY();
        } else {
            qCoord = query.getZ() - center.getZ();
            hRadius = leafRadii.getZ();
        }

        if ((qCoord - radius < -hRadius - eps) && (qCoord + radius > hRadius + eps)) {
            continue;
        }


        const auto& keys = reordered.getLeafKeys(leaf, order);
        if (keys.empty()) {
            continue;
        }

        const double firstKey = keys.front();
        const double lastKey  = keys.back();

        const double kMin = qCoord - radius;
        const double kMax = qCoord + radius;

        size_t iMin = 0;
        if (kMin > firstKey) {
            // iMin = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMin) - keys.begin());
            iMin = branchless_lower_bound (keys, kMin);
        }

        size_t iMax = count;
        if (kMax < lastKey) {
            // iMax = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMax) - keys.begin());
            iMax = branchless_upper_bound (keys, kMax);
        }

        // 5. EVALUACIÓN DEL MEJOR RANGO
        PrunedRange r{iMin, iMax, 0, 0, false, order};
        const size_t rangeCount = r.count();

        if (rangeCount < best.count()) {
            best = r;
            // Early exit si la poda es lo suficientemente agresiva
            if ((static_cast<double>(rangeCount) / count) <= UMBRAL_PCT_PODA) {
                return best;
            }
        }
    }

    return best;
}

// ###########################################################################################
// ############################ MODO POLAR ##################################
// ###########################################################################################
template<typename Octree_t, typename Reordered_t>
inline PrunedRange bestRangePolar(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    size_t count,
    const Octree_t& octree,
    const Reordered_t& reordered)
{
    PrunedRange full{0, count, 0, 0, false, OrderType::K0};
    constexpr double eps = 1e-9;

    const Point& center = octree.getLeafCenter(leaf);
    
    double dx = 0.0;
    double dy = 0.0;
    double dxy = 0.0;
    double rxyEff = 0.0;
    double phiQ = 0.0;
    double deltaPhi = 0.0;

    dx = query.getX() - center.getX();
    dy = query.getY() - center.getY();

    dxy = std::sqrt(dx * dx + dy * dy);
    if (dxy <= eps) return full;

    rxyEff = detail::effectiveXYRadius(radius, kernel);
    if (dxy < rxyEff) return full;

    phiQ = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
    deltaPhi = std::asin(std::clamp(rxyEff / dxy, 0.0, 1.0));
    if (deltaPhi >= detail::kPi) return full;

    const double kMinRaw = phiQ - deltaPhi;
    const double kMaxRaw = phiQ + deltaPhi;

    if (kMinRaw < 0.0 || kMaxRaw >= detail::kTwoPi) {
        return full;
    }

    const auto& keys = reordered.getLeafKeys(leaf, OrderType::K0);
    if (keys.empty()) {
        return full;
    }

    // if (kMinRaw < 0.0) {
    //     size_t iMax1 = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw) - keys.begin());
    //     size_t iMin2 = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw + detail::kTwoPi) - keys.begin());
    //     if (mainOptions.debugLeavesTime) {
    //         accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
    //     }
    //     return {0, iMax1, iMin2, count, true, OrderType::K0};
    // }
    
    // if (kMaxRaw >= detail::kTwoPi) {
    //     size_t iMax1 = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw - detail::kTwoPi) - keys.begin());
    //     size_t iMin2 = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw) - keys.begin());
    //     if (mainOptions.debugLeavesTime) {
    //         accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
    //     }
    //     return {0, iMax1, iMin2, count, true, OrderType::K0};
    // }

    // size_t iMin = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw) - keys.begin());
    // size_t iMax = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw) - keys.begin());
    size_t iMin = branchless_lower_bound (keys, kMinRaw);
    size_t iMax = branchless_upper_bound (keys, kMaxRaw);
    
    return {iMin, iMax, 0, 0, false, OrderType::K0};
}












// ###########################################################################################
// ########################## MODOS DEBUG  ###############################
// ###########################################################################################
template<typename Octree_t, typename Reordered_t>
inline PrunedRange bestRangeCartesianDebug(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    size_t count,
    const Octree_t& octree,
    const Reordered_t& reordered,
    const Vector& leafRadii)
{
    const Point& center = octree.getLeafCenter(leaf);
    PrunedRange best{0, count, 0, 0, false, OrderType::K0};
    const double eps = 1e-9;
    // Evaluamos los 3 ejes de forma directa
    for (OrderType order : {OrderType::K0, OrderType::K1, OrderType::K2}) {
        double qCoord = 0.0;
        double hRadius = 0.0;
        if (mainOptions.debugLeavesTime) {
            const auto projectionStart = std::chrono::steady_clock::now();
            if (order == OrderType::K0) {
                qCoord = query.getX() - center.getX();
                hRadius = leafRadii.getX();
            } else if (order == OrderType::K1) {
                qCoord = query.getY() - center.getY();
                hRadius = leafRadii.getY();
            } else {
                qCoord = query.getZ() - center.getZ();
                hRadius = leafRadii.getZ();
            }

            if ((qCoord - radius < -hRadius - eps) && (qCoord + radius > hRadius + eps)) {
                accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
                continue;
            }
            accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
        } else {
            if (order == OrderType::K0) {
                qCoord = query.getX() - center.getX();
                hRadius = leafRadii.getX();
            } else if (order == OrderType::K1) {
                qCoord = query.getY() - center.getY();
                hRadius = leafRadii.getY();
            } else {
                qCoord = query.getZ() - center.getZ();
                hRadius = leafRadii.getZ();
            }

            if ((qCoord - radius < -hRadius - eps) && (qCoord + radius > hRadius + eps)) {
                continue;
            }
        }

        const auto binaryStart = std::chrono::steady_clock::now();
        const auto& keys = reordered.getLeafKeys(leaf, order);
        if (keys.empty()) {
            if (mainOptions.debugLeavesTime) {
                accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
            }
            continue;
        }

        const double firstKey = keys.front();
        const double lastKey  = keys.back();

        const double kMin = qCoord - radius;
        const double kMax = qCoord + radius;

        size_t iMin = 0;
        if (kMin > firstKey) {
            // iMin = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMin) - keys.begin());
            iMin = branchless_lower_bound (keys, kMin);
        }

        size_t iMax = count;
        if (kMax < lastKey) {
            // iMax = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMax) - keys.begin());
            iMax = branchless_upper_bound (keys, kMax);
        }

        if (mainOptions.debugLeavesTime) {
            accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
        }

        // 5. EVALUACIÓN DEL MEJOR RANGO
        PrunedRange r{iMin, iMax, 0, 0, false, order};
        const size_t rangeCount = r.count();

        if (rangeCount < best.count()) {
            best = r;
            // Early exit si la poda es lo suficientemente agresiva
            if ((static_cast<double>(rangeCount) / count) <= UMBRAL_PCT_PODA) {
                return best;
            }
        }
    }

    return best;
}

// ###########################################################################################
// ############################ MODO POLAR ##################################
// ###########################################################################################
template<typename Octree_t, typename Reordered_t>
inline PrunedRange bestRangePolarDebug(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    size_t count,
    const Octree_t& octree,
    const Reordered_t& reordered)
{
    PrunedRange full{0, count, 0, 0, false, OrderType::K0};
    constexpr double eps = 1e-9;

    const Point& center = octree.getLeafCenter(leaf);
    
    double dx = 0.0;
    double dy = 0.0;
    double dxy = 0.0;
    double rxyEff = 0.0;
    double phiQ = 0.0;
    double deltaPhi = 0.0;

    if (mainOptions.debugLeavesTime) {
        const auto projectionStart = std::chrono::steady_clock::now();
        dx = query.getX() - center.getX();
        dy = query.getY() - center.getY();

        dxy = std::sqrt(dx * dx + dy * dy);
        if (dxy <= eps) {
            accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
            return full;
        }

        rxyEff = detail::effectiveXYRadius(radius, kernel);
        if (dxy < rxyEff) {
            accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
            return full;
        }

        phiQ = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
        deltaPhi = std::asin(std::clamp(rxyEff / dxy, 0.0, 1.0));
        if (deltaPhi >= detail::kPi) {
            accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
            return full;
        }

        accumulatedProjectionTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - projectionStart).count();
    } else {
        dx = query.getX() - center.getX();
        dy = query.getY() - center.getY();

        dxy = std::sqrt(dx * dx + dy * dy);
        if (dxy <= eps) return full;

        rxyEff = detail::effectiveXYRadius(radius, kernel);
        if (dxy < rxyEff) return full;

        phiQ = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
        deltaPhi = std::asin(std::clamp(rxyEff / dxy, 0.0, 1.0));
        if (deltaPhi >= detail::kPi) return full;
    }

    const double kMinRaw = phiQ - deltaPhi;
    const double kMaxRaw = phiQ + deltaPhi;

    const auto binaryStart = std::chrono::steady_clock::now();
    if (kMinRaw < 0.0 || kMaxRaw >= detail::kTwoPi) {
        if (mainOptions.debugLeavesTime) {
            accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
        }
        return full;
    }
    const auto& keys = reordered.getLeafKeys(leaf, OrderType::K0);
    if (keys.empty()) {
        if (mainOptions.debugLeavesTime) {
            accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
        }
        return full;
    }

    // if (kMinRaw < 0.0) {
    //     size_t iMax1 = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw) - keys.begin());
    //     size_t iMin2 = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw + detail::kTwoPi) - keys.begin());
    //     if (mainOptions.debugLeavesTime) {
    //         accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
    //     }
    //     return {0, iMax1, iMin2, count, true, OrderType::K0};
    // }
    
    // if (kMaxRaw >= detail::kTwoPi) {
    //     size_t iMax1 = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw - detail::kTwoPi) - keys.begin());
    //     size_t iMin2 = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw) - keys.begin());
    //     if (mainOptions.debugLeavesTime) {
    //         accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
    //     }
    //     return {0, iMax1, iMin2, count, true, OrderType::K0};
    // }

    // size_t iMin = static_cast<size_t>(std::lower_bound(keys.begin(), keys.end(), kMinRaw) - keys.begin());
    // size_t iMax = static_cast<size_t>(std::upper_bound(keys.begin(), keys.end(), kMaxRaw) - keys.begin());
    size_t iMin = branchless_lower_bound (keys, kMinRaw);
    size_t iMax = branchless_upper_bound (keys, kMaxRaw);

    if (mainOptions.debugLeavesTime) {
        accumulatedBinarySearchTime += std::chrono::duration<double>(std::chrono::steady_clock::now() - binaryStart).count();
    }

    return {iMin, iMax, 0, 0, false, OrderType::K0};
}
