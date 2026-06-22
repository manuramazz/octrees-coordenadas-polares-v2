#pragma once

#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <omp.h>

#include "octree_types.hpp"
#include "main_options.hpp"

/** 
 * @class OctreeReordered
 *
 * @brief This class calculates and stores the reordered point data for each leaf of the octree based on the specified reordering mode (Cartesian or Polar). It also computes the keys for binary search in the search process.
 *
 * @details The class provides two main functionalities:
 * 1. Calculating the keys of the points in each leaf separately based on the selected reordering mode. For Cartesian mode, it calculates the keys based on the X, Y, and Z coordinates. For Polar mode, it calculates the keys based on the azimuthal angle in the XY plane.
 *    The keys are stored in a structure of vectors that allows for efficient access during the search process.
 * 2. Building the reordered point data flat structure that contains the points in the order defined by the permutations of the keys. 
 *    The start of each leaf in this reordered structure corresponds to the original global index of the points in the octree.
 */
template<typename Octree_t, typename Container>
class OctreeReordered {
public:

    struct LeafPermutations {
        std::array<std::vector<size_t>, 3> perms;
    };
    struct LeafKeys {
        std::array<std::vector<double>, 3> keys;
    };

    
    std::vector<LeafKeys> leafKeys;

    // ========================================================================
    // Build of the reordered vectors (1:1 with original points, no compaction)
    // =========================================================================
    SortedDataFlat sortedFlat;
    
    void buildSortedDataFlat(
        Octree_t& octree,
        Container& points,
        ReorderMode mode,
        std::vector<LeafPermutations>& leafPerms)
    {
        if (mode == ReorderMode::None)
            return;

        const size_t numLeaves = octree.getNumLeaves();
        const size_t totalPoints = points.size();

        // The building of the sorted data is flexible to permit being used with pointer octrees as well as linear octrees.
        constexpr bool useLeafPoints = requires(Octree_t& o, size_t l) { o.getLeafPoints(l); };

        if (mode == ReorderMode::Polar) {
            sortedFlat.PointsPolar.resize(totalPoints);
        } else if (mode == ReorderMode::Cartesian) {
            sortedFlat.PointsX.resize(totalPoints);
            sortedFlat.PointsY.resize(totalPoints);
            sortedFlat.PointsZ.resize(totalPoints);
        }

        // Parallel building of the sorted data flat structure
        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            size_t count = 0;
            size_t begin = 0;
            std::vector<size_t> leafPointsLocal;

            if constexpr (useLeafPoints) {
                leafPointsLocal = octree.getLeafPoints(leaf);
                count = leafPointsLocal.size();
            } else {
                auto [b, e] = octree.getLeafRange(leaf);
                begin = b;
                count = e - b;
            }

            if (count == 0) continue;

            // CASE A:  number of leaf points > umbralPoda) -> Reorder points
            if (count > mainOptions.umbralPoda) 
            {
                if (mode == ReorderMode::Cartesian) {
                    // Cartesian mode: for each axis, we reorder the points according to the permutations calculated previously
                    for (size_t i = 0; i < count; ++i) {
                        size_t globalIdx = useLeafPoints ? leafPointsLocal[i] : (begin + i);

                        size_t idxX = useLeafPoints ? leafPointsLocal[leafPerms[leaf].perms[0][i]] : (begin + leafPerms[leaf].perms[0][i]);
                        size_t idxY = useLeafPoints ? leafPointsLocal[leafPerms[leaf].perms[1][i]] : (begin + leafPerms[leaf].perms[1][i]);
                        size_t idxZ = useLeafPoints ? leafPointsLocal[leafPerms[leaf].perms[2][i]] : (begin + leafPerms[leaf].perms[2][i]);

                        const auto& pX = points[idxX];
                        const auto& pY = points[idxY];
                        const auto& pZ = points[idxZ];

                        // We need to store the original global index in the id field to be able to retrieve the original point data during the search process
                        sortedFlat.PointsX[globalIdx] = Point(idxX, pX.getX(), pX.getY(), pX.getZ());
                        sortedFlat.PointsY[globalIdx] = Point(idxY, pY.getX(), pY.getY(), pY.getZ());
                        sortedFlat.PointsZ[globalIdx] = Point(idxZ, pZ.getX(), pZ.getY(), pZ.getZ());
                    }
                } 
                else {
                    // Polar mode: for each leaf, we reorder according to the azimuthal angle values calculated previously.
                    for (size_t i = 0; i < count; ++i) {
                        size_t globalIdx = useLeafPoints ? leafPointsLocal[i] : (begin + i);

                        size_t idxPolar = useLeafPoints ? leafPointsLocal[leafPerms[leaf].perms[0][i]] : (begin + leafPerms[leaf].perms[0][i]);

                        const auto& p = points[idxPolar];
                        sortedFlat.PointsPolar[globalIdx] = Point(p.getX(), p.getY(), p.getZ());
                    }
                }
            }
            // CASE B: number of leaf points <= umbralPoda) -> Direct copy
            else 
            {
                for (size_t i = 0; i < count; ++i) {
                    size_t globalIdx = useLeafPoints ? leafPointsLocal[i] : (begin + i);
                    const auto& p = points[globalIdx];

                    if (mode == ReorderMode::Cartesian) {
                        Point pt(globalIdx, p.getX(), p.getY(), p.getZ());
                        sortedFlat.PointsX[globalIdx] = pt;
                        sortedFlat.PointsY[globalIdx] = pt;
                        sortedFlat.PointsZ[globalIdx] = pt;
                    } else {
                        sortedFlat.PointsPolar[globalIdx] = Point(p.getX(), p.getY(), p.getZ());
                    }
                }
            }
        }
        std::cout << "Finished building sorted data flat 1:1 with " << totalPoints 
                << " points. Mode: " << localReorderTypeToString(mode) << std::endl;
    }

    const SortedDataFlat& getSortedFlat() const {
        return sortedFlat;
    }

    // ============================================================
    // Building of permutation and keys vectors
    // ============================================================
    void buildLeafPermutations(
        Octree_t& octree,
        Container& points,
        ReorderMode mode)
    {
        if (mode == ReorderMode::None)
            return;

        size_t numLeaves = octree.getNumLeaves();
        std::vector<LeafPermutations> leafPerms;

        leafPerms.resize(numLeaves);
        leafKeys.resize(numLeaves);

        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            size_t count = 0;
            auto leafPoints = std::vector<size_t>{};
            size_t begin = 0;
            size_t end = 0;
            // 
            if constexpr (requires { octree.getLeafPoints(leaf); }) {
                leafPoints = octree.getLeafPoints(leaf);
                count = leafPoints.size();
            }
            else{
                auto range = octree.getLeafRange(leaf);
                begin = range.first;
                end   = range.second;
                count = end - begin;
            }

            if(count <= mainOptions.umbralPoda) {
                // No reorder for small leaves, to avoid overhead of keys and sorts
                continue;
            }

            const auto& center = octree.getLeafCenter(leaf);
            size_t nVectors = (mode == ReorderMode::Polar) ? 1 : 3;
            // inicializar permutaciones y claves
            for (int k = 0; k < nVectors; ++k) {
                leafKeys[leaf].keys[k].resize(count);
                leafPerms[leaf].perms[k].resize(count);
                std::iota(leafPerms[leaf].perms[k].begin(),
                          leafPerms[leaf].perms[k].end(), 0);
            }

            // --------------------------------
            // Key calcutation
            // --------------------------------
            for (size_t i = 0; i < count; ++i)
            {
                // We need to obtain the global index of all points to access their coordinates
                
                size_t idx = begin + i;
                if constexpr (requires { octree.getLeafPoints(leaf); }) {
                    idx = leafPoints[i];
                }
                double dx, dy, dz;
                if constexpr (std::is_same_v<Container, PointsSoA>) {
                    dx = points.dataX()[idx] - center.getX();
                    dy = points.dataY()[idx] - center.getY();
                    dz = points.dataZ()[idx] - center.getZ();
                } else {
                    dx = points[idx].getX() - center.getX();
                    dy = points[idx].getY() - center.getY();
                    dz = points[idx].getZ() - center.getZ();
                }

                // angle in XY plane [0, 2pi)
                if (mode == ReorderMode::Polar)
                {
                    const double phi = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
                    const double rxy = std::sqrt(dx * dx + dy * dy);
                    leafKeys[leaf].keys[0][i] = phi;
                }
                // Cartesian mode: X, Y and Z axis
                else if (mode == ReorderMode::Cartesian) 
                {
                    leafKeys[leaf].keys[0][i] = dx;
                    leafKeys[leaf].keys[1][i] = dy;
                    leafKeys[leaf].keys[2][i] = dz;
                }
            }

            // --------------------------------
            // order permutations by keys
            // --------------------------------
            std::vector<double> sortedK(count);
            for (int k = 0; k < nVectors; ++k)
            {
                auto& perm = leafPerms[leaf].perms[k];

                std::sort(
                    perm.begin(),
                    perm.end(),
                    [&](size_t a, size_t b) {
                        return leafKeys[leaf].keys[k][a] < leafKeys[leaf].keys[k][b];
                });
                
                for (size_t i = 0; i < count; ++i)
                    sortedK[i] = leafKeys[leaf].keys[k][perm[i]];
                leafKeys[leaf].keys[k] = sortedK;
            }
        }
        // Once perms and keys are built, we can build the sorted vectors
        buildSortedDataFlat(octree, points, mode, leafPerms);
    }

    // ==========================
    // access to keys
    // ==========================
    const std::vector<double>& getLeafKeys(size_t leaf, OrderType type) const {
         return leafKeys[leaf].keys[static_cast<int>(type)];
    }

};