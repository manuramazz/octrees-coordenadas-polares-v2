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

/**
 * @brief Operations for selecting the range of points to evaluate in a leaf based on the query and the kernel.
 * 
 * @details Includes two main functions. The first one, `bestRangeCartesian`, evaluates the 3 coordinate axes independently and selects the best range of points to evaluate based on the smallest range obtained from any of the axes. 
 * The second one, `bestRangePolar`, uses polar coordinates. Each point is defined by its azimuthal angle from the leaf center. 
 * The function computes the angular range that could contain points based on the position and radius of the query, and then finds the corresponding range of points in the leaf.
 * Both functions return a `PrunedRange` structure that contains the indices of the points to evaluate and additional information about the pruning process.
 * 
 */



/** 
* @brief Binary search fuction, it calculates the lower bound of the range that will be evaluated.
* 
* @param keys vector of keys to search in
* @param val value to search for in the keys
* @return Index of the first element that is not less than val.
*/
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

/** 
* @brief Binary search fuction, it calculates the upper bound of the range that will be evaluated.
* 
* @param keys vector of keys to search in
* @param val value to search for in the keys
* @return Index of the last element that is less than val.
*/
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
// ###################################### CARTESIAN MODE  ####################################
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
    const double eps = 1e-9;

    OrderType bestOrder = OrderType::K0;
    double minIntervalSpan = std::numeric_limits<double>::max();
    double bestKMin = 0.0;
    double bestKMax = 0.0;
    bool anyAxisPruned = false;

    // Loop to find the best axis for pruning based on the smallest space interval span
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

        const double kMin = qCoord - radius;
        const double kMax = qCoord + radius;
        

        double currentSpan = kMax - kMin;

        if (currentSpan < minIntervalSpan) {
            minIntervalSpan = currentSpan;
            bestKMin = kMin;
            bestKMax = kMax;
            bestOrder = order;
            anyAxisPruned = true;
        }
    }

    // No pruning -> return full 
    if (!anyAxisPruned) {
        return {0, count, 0, 0, false, OrderType::K0};
    }

    // Once selected the best axis, we access their keys
    const auto& keys = reordered.getLeafKeys(leaf, bestOrder);
    if (keys.empty()) {
        return {0, count, 0, 0, false, bestOrder};
    }

    const double firstKey = keys.front();
    const double lastKey  = keys.back();

    // Binary searches on the best axis (if needed)
    size_t iMin = 0;
    if (bestKMin > firstKey) {
        iMin = branchless_lower_bound(keys, bestKMin);
    }

    size_t iMax = count;
    if (bestKMax < lastKey) {
        iMax = branchless_upper_bound(keys, bestKMax);
    }

    return {iMin, iMax, 0, 0, false, bestOrder};
}


// ###########################################################################################
// ################################### POLAR MODE ############################################
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

    // Min and max angles for the range: angle of the query point from the octant center ± deltaPhi, normalized to [0, 2π)
    const double kMinRaw = phiQ - deltaPhi;
    const double kMaxRaw = phiQ + deltaPhi;

    const auto& keys = reordered.getLeafKeys(leaf, OrderType::K0);
    if (keys.empty()) {
        return full;
    }

    // If the range crosses the 0/2π boundary, the angle is wrapped around in two ranges.
    if (kMinRaw < 0.0) {
        size_t iMax1 = branchless_upper_bound(keys, kMaxRaw);
        size_t iMin2 = branchless_lower_bound(keys, kMinRaw + detail::kTwoPi);
        return {0, iMax1, iMin2, count, true, OrderType::K0};
    }
    
    if (kMaxRaw >= detail::kTwoPi) {
        size_t iMax1 = branchless_upper_bound(keys, kMaxRaw - detail::kTwoPi);
        size_t iMin2 = branchless_lower_bound(keys, kMinRaw);

        return {0, iMax1, iMin2, count, true, OrderType::K0};
    }

    size_t iMin = branchless_lower_bound (keys, kMinRaw);
    size_t iMax = branchless_upper_bound (keys, kMaxRaw);

    return {iMin, iMax, 0, 0, false, OrderType::K0};
}


