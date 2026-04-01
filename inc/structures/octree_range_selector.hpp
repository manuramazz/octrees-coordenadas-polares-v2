#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "../geometry/point.hpp"
#include "../main_options.hpp"
#include "octree_types.hpp"

namespace detail {

inline constexpr double kPi    = 3.14159265358979323846;
inline constexpr double kTwoPi = 2.0 * kPi;

inline double normalizeAngle0To2Pi(double a) {
    double out = std::fmod(a, kTwoPi);
    return out < 0.0 ? out + kTwoPi : out;
}

inline double effectiveRadius(double radius, Kernel_t kernel) {
    return (kernel == Kernel_t::cube || kernel == Kernel_t::square)
        ? radius * std::sqrt(3.0)
        : radius;
}

// Devuelve la clave de un punto (índice local i en la hoja) para un orden dado.
// Recibe las coordenadas relativas al centro de la hoja ya calculadas.
inline double computePointKey(
    double dx, double dy, double dz,
    double rxy,
    OrderType order,
    ReorderMode mode)
{
    if (order == OrderType::K0) {
        return normalizeAngle0To2Pi(std::atan2(dy, dx));
    }
    if (order == OrderType::K1) {
        if (mode == ReorderMode::Spherical) {
            const double r = std::sqrt(rxy * rxy + dz * dz);
            return (r > 0.0) ? std::acos(std::clamp(dz / r, -1.0, 1.0)) : 0.0;
        }
        return rxy; // cilíndrico
    }
    // K2
    if (mode == ReorderMode::Spherical)
        return std::sqrt(rxy * rxy + dz * dz);
    return dz; // cilíndrico
}

} // namespace detail

// ---------------------------------------------------------------



// Calcula el rango [iMin, iMax) sobre la permutación de `order` para una hoja.
// No requiere caché: recalcula las claves de cada punto cuando hace lower/upper_bound.
template<typename Octree_t, typename Container, typename Reordered_t>
PrunedRange computeRange(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    OrderType order,
    const Octree_t& octree,
    const Container& points,
    const Reordered_t& reordered,
    ReorderMode mode,
    bool logging = false)
{
    const auto [begin, end] = octree.getLeafRange(leaf);
    const size_t count = end - begin;
    PrunedRange full{0, count, order};

    if (count == 0 || mode == ReorderMode::None)
        return full;

    const Point& center = octree.getLeafCenter(leaf);
    const double dx  = query.getX() - center.getX();
    const double dy  = query.getY() - center.getY();
    const double dz  = query.getZ() - center.getZ();
    const double dxy = std::sqrt(dx * dx + dy * dy);
    const double d   = std::sqrt(dxy * dxy + dz * dz);
    const double rEff = detail::effectiveRadius(radius, kernel);

    constexpr double eps = 1e-12;

    // Calcula la clave del i-ésimo punto en el orden de la permutación
    const auto& perm = reordered.getLeafPermutation(leaf, order);
    auto keyAt = [&](size_t permIdx) -> double {
        const size_t globalIdx = begin + perm[permIdx];
        double px, py, pz;
        if constexpr (std::is_same_v<Container, PointsSoA>) {
            px = points.dataX()[globalIdx] - center.getX();
            py = points.dataY()[globalIdx] - center.getY();
            pz = points.dataZ()[globalIdx] - center.getZ();
        } else {
            px = points[globalIdx].getX() - center.getX();
            py = points[globalIdx].getY() - center.getY();
            pz = points[globalIdx].getZ() - center.getZ();
        }
        const double prxy = std::sqrt(px * px + py * py);
        return detail::computePointKey(px, py, pz, prxy, order, mode);
    };

    // lower/upper bound sobre la permutación usando keyAt
    auto lowerIdx = [&](double val) -> size_t {
        size_t lo = 0, hi = count;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (keyAt(mid) < val) lo = mid + 1; else hi = mid;
        }
        return lo;
    };
    auto upperIdx = [&](double val) -> size_t {
        size_t lo = 0, hi = count;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (keyAt(mid) <= val) lo = mid + 1; else hi = mid;
        }
        return lo;
    };

    if (order == OrderType::K2) {
        if (mode == ReorderMode::Spherical) {
            return {lowerIdx(std::max(0.0, d - rEff)), upperIdx(d + rEff), order};
        }
        return {lowerIdx(dz - rEff), upperIdx(dz + rEff), order};
    }

    if (order == OrderType::K1) {
        if (mode == ReorderMode::Spherical) {
            if (d <= eps) return full;
            const double thetaQ    = std::acos(std::clamp(dz / d, -1.0, 1.0));
            const double deltaTheta = std::asin(std::clamp(rEff / d, 0.0, 1.0));
            return {lowerIdx(std::max(0.0, thetaQ - deltaTheta)),
                    upperIdx(std::min(detail::kPi, thetaQ + deltaTheta)), order};
        }
        return {lowerIdx(std::max(0.0, dxy - rEff)), upperIdx(dxy + rEff), order};
    }

    // K0 azimutal
    if (dxy <= eps) return full;
    const double phiQ     = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
    const double deltaPhi = std::asin(std::clamp(rEff / dxy, 0.0, 1.0));
    if (deltaPhi >= detail::kPi) return full;

    const double kMinRaw = phiQ - deltaPhi;
    const double kMaxRaw = phiQ + deltaPhi;

    if (kMinRaw < 0.0 && kMaxRaw >= detail::kTwoPi)
        return full;                                  // el rango cubre todo el círculo 
    /*if (kMinRaw < 0.0)
        return {0, upperIdx(kMaxRaw), order};          // aprovecha la mitad alta
    if (kMaxRaw >= detail::kTwoPi)
        return {lowerIdx(kMinRaw), count, order};      // aprovecha la mitad baja
    */
    return {lowerIdx(kMinRaw), upperIdx(kMaxRaw), order};
}

template<typename Octree_t, typename Container, typename Reordered_t>
PrunedRange bestRange(
    size_t leaf,
    const Point& query,
    double radius,
    Kernel_t kernel,
    const Octree_t& octree,
    const Container& points,
    const Reordered_t& reordered,
    ReorderMode mode,
    bool logging = true)
{
    const auto [begin, end] = octree.getLeafRange(leaf);
    PrunedRange best{0, end - begin, OrderType::K0};

    for (OrderType order : {OrderType::K0, OrderType::K1, OrderType::K2}) {
        PrunedRange r = computeRange(
            leaf, query, radius, kernel, order,
            octree, points, reordered, mode, logging);
        if (r.count() < best.count())
            best = r;
        if (logging) {
            std::ostringstream oss;
            oss << "Leaf " << leaf << " order=" << static_cast<int>(order)
                 << " range=[" << r.iMin << "," << r.iMax << ") count=" << r.count();
            std::cout << oss.str() << '\n';
        }

    }
    if (logging) {
        std::cout << "Leaf " << leaf << ": best order=" << static_cast<int>(best.order)
                  << " count=" << best.count() << " / " << (end - begin) << '\n';
    }

    return best;
}