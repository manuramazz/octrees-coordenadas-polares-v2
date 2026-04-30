#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "../geometry/point.hpp"
#include "../main_options.hpp"
#include "octree_types.hpp"

// ---------------------------------------------------------------

bool headers = false;

inline std::ofstream& getRangeSelectorLog() {
    static std::ofstream logFile = []() {
        const std::string baseName =
            mainOptions.inputFileName.empty() ? "input" : mainOptions.inputFileName;

        const auto now = std::chrono::system_clock::now();
        const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm tmSnapshot{};
#ifdef _WIN32
        localtime_s(&tmSnapshot, &nowTime);
#else
        localtime_r(&nowTime, &tmSnapshot);
#endif

        std::ostringstream stamp;
        stamp << std::put_time(&tmSnapshot, "%Y%m%d_%H%M%S");

        const std::filesystem::path logDir = mainOptions.outputDirName / "ranges";
        std::error_code ec;
        std::filesystem::create_directories(logDir, ec);

        const std::filesystem::path logPath = logDir / (baseName + "_" + stamp.str() + ".log");
        return std::ofstream(logPath, std::ios::app);
    }();
    
    return logFile;
}

inline void printLog(const std::string& message) {
    auto& f = getRangeSelectorLog();
    if(!headers) {
        f << "leaf,kernel,mode,radius,count,total,key\n";
        headers = true;
    }
    if (f.is_open())
        f << message << '\n';
}

// Calcula el rango [iMin, iMax) sobre la permutación de `order` para una hoja.

// Deprecated: No requiere caché: recalcula las claves de cada punto cuando hace lower/upper_bound.
//VERSIÓN NUEVA: la reordenación guarda caché de claves, en la búsqueda solo se leen los vectores de memoria
template<typename Octree_t, typename Reordered_t>
PrunedRange computeRange(
    size_t leaf,
    Kernel_t kernel,
    OrderType order,
    const Octree_t& octree,
    const size_t count,
    const Reordered_t& reordered,
    ReorderMode mode,
    const detail::LeafQueryGeometry& geo )
{
    
    PrunedRange full{0, count, 0,0,false, order};

    if (count <= 1 || mode == ReorderMode::None)
        return full;
    constexpr double eps = 1e-12;

    const auto& keys = reordered.getLeafKeys(leaf, order);

    // lowerIdx: primer índice donde keys[mid] >= val
    auto lowerIdx = [&](double val) -> size_t {
        return static_cast<size_t>(
            std::lower_bound(keys.begin(), keys.end(), val) - keys.begin());
    };

    // upperIdx: primer índice donde keys[mid] > val
    auto upperIdx = [&](double val) -> size_t {
        return static_cast<size_t>(
            std::upper_bound(keys.begin(), keys.end(), val) - keys.begin());
    };


    // Rangos según clave y kernel
    if (order == OrderType::K2) {
        if (mode == ReorderMode::Spherical) {
            return {lowerIdx(std::max(0.0, geo.d - geo.rEff)), upperIdx(geo.d + geo.rEff),0,0,false, order};
        }
        
        return {lowerIdx(geo.dz - geo.radius), upperIdx(geo.dz + geo.radius),0,0,false, order};
    }

    if (order == OrderType::K1) {
        if (mode == ReorderMode::Spherical) {
            if (geo.d <= eps) return full;
            if (geo.rEff >= geo.d) return full;
            const double thetaQ    = std::acos(std::clamp(geo.dz / geo.d, -1.0, 1.0));
            const double deltaTheta = std::asin(std::clamp(geo.rxyEff / geo.d, 0.0, 1.0));
            return {lowerIdx(std::max(0.0, thetaQ - deltaTheta)),
                    upperIdx(std::min(detail::kPi, thetaQ + deltaTheta)),0,0,false, order};
        }
        return {lowerIdx(std::max(0.0, geo.dxy - geo.rxyEff)), upperIdx(geo.dxy + geo.rxyEff),0,0,false, order};
    }

    // K0 azimutal
    if (geo.dxy <= eps) return full;
    if (geo.dxy < geo.rxyEff) return full;
    const double phiQ     = detail::normalizeAngle0To2Pi(std::atan2(geo.dy, geo.dx));
    const double deltaPhi = std::asin(std::clamp(geo.rxyEff / geo.dxy, 0.0, 1.0));
    if (deltaPhi >= detail::kPi) return full;

    const double kMinRaw = phiQ - deltaPhi;
    const double kMaxRaw = phiQ + deltaPhi;

    if (kMinRaw < 0.0) {
        // Wrap por la izquierda: [0, kMaxRaw] ∪ [kMinRaw+2π, 2π)
        const size_t iMax1 = upperIdx(kMaxRaw);
        const size_t iMin2 = lowerIdx(kMinRaw + detail::kTwoPi);
        return {0, iMax1, iMin2, count, true, order};
    }
    if (kMaxRaw >= detail::kTwoPi) {
        // Wrap por la derecha: [0, kMaxRaw-2π] ∪ [kMinRaw, 2π)
        const size_t iMax1 = upperIdx(kMaxRaw - detail::kTwoPi);
        const size_t iMin2 = lowerIdx(kMinRaw);
        return {0, iMax1, iMin2, count, true, order};
    }

    return {lowerIdx(kMinRaw), upperIdx(kMaxRaw), 0, 0, false, order};
}

template<typename Octree_t, typename Reordered_t>
PrunedRange bestRange(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    const Octree_t& octree,
    const Reordered_t& reordered,
    ReorderMode mode,
    bool logging = false)
{
    const size_t count = reordered.getLeafPermutation(leaf, OrderType::K0).size();
    const Point& center = octree.getLeafCenter(leaf);
    const auto geo = detail::LeafQueryGeometry::compute(query, center, radius, kernel);

    PrunedRange best{0, count, 0,0,false, OrderType::K0};

    for (OrderType order : {OrderType::K0, OrderType::K1, OrderType::K2}) {
        PrunedRange r = computeRange(leaf, kernel, order, octree, count, reordered, mode, geo);
        if (r.count() < best.count())
            best = r;
    }
    if (mainOptions.debugRanges) {
        printLog(std::to_string(leaf) + "," + std::string(kernelToString(kernel)) +","+ std::string(localReorderTypeToString(mode)) + "," + std::to_string(radius) + "," + std::to_string(best.count()) + "," + std::to_string(count) + "," +std::to_string(static_cast<int>(best.order)));
    }
    if (logging)
        std::cout << "Leaf " << leaf << ": best order=" << static_cast<int>(best.order)
                  << " count=" << best.count() << " / " << count << '\n';

    return best;
}