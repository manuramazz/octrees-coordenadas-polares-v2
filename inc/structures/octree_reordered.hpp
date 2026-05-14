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

    // ============================================================
    // Función de construcción de array plano reordenado (duplicación)
    // ============================================================
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
        sortedFlat.isPolar = (mode == ReorderMode::Polar);
        // Paso 1 — calcular offsets (necesita saber el count de cada hoja)
        sortedFlat.leafOffsets.resize(numLeaves + 1);
        sortedFlat.leafOffsets[0] = 0;

        for (size_t leaf = 0; leaf < numLeaves; ++leaf) {
            size_t count = leafPerms[leaf].perms[0].size();
            if(count <= mainOptions.umbralPoda) count = 0;
            sortedFlat.leafOffsets[leaf + 1] = sortedFlat.leafOffsets[leaf] + count;
        }

        const size_t totalPoints = sortedFlat.leafOffsets[numLeaves];
        sortedFlat.allData.resize(
            mode == ReorderMode::Cartesian ? totalPoints * 3 : totalPoints
        );

        // Paso 2 — rellenar en paralelo (cada hoja escribe en su propio rango, sin solapamiento)
        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            const size_t offset     = sortedFlat.leafOffsets[leaf];
            const size_t nextOffset = sortedFlat.leafOffsets[leaf + 1];
            if (offset == nextOffset) continue;
            const size_t count = nextOffset - offset;

            size_t begin = 0;
            std::vector<size_t> leafPointsLocal;

            if constexpr (requires { octree.getLeafPoints(leaf); }) {
                leafPointsLocal = octree.getLeafPoints(leaf);
            } else {
                auto [b, e] = octree.getLeafRange(leaf);
                begin = b;
            }

            if (mode == ReorderMode::Cartesian) {
                // 3 órdenes: X, Y, Z — cada uno ocupa su propio bloque en allData
                // Layout: [bloque_X | bloque_Y | bloque_Z] para cada hoja
                // Los offsets apuntan al inicio del bloque X; Y y Z están desplazados
                // Para simplificar, aquí llenamos los 3 bloques contiguos:
                // allData[offset*3 + 0..count)   = ordenado por X
                // allData[offset*3 + count..2*count) = ordenado por Y
                // allData[offset*3 + 2*count..3*count) = ordenado por Z

                for (int axis = 0; axis < 3; ++axis) {
                    const auto& perm = leafPerms[leaf].perms[axis];
                    const size_t blockOffset = offset * 3 + axis * count;

                    for (size_t i = 0; i < count; ++i) {
                        size_t globalIdx;
                        if constexpr (requires { octree.getLeafPoints(leaf); }) {
                            globalIdx = leafPointsLocal[perm[i]];
                        } else {
                            globalIdx = begin + perm[i];
                        }
                        const auto& p = points[globalIdx];
                        sortedFlat.allData[blockOffset + i] = Point(globalIdx, p.getX(), p.getY(), p.getZ());
   
                        
                    }
                }

            } else {
                // Polar: un solo orden (K0)
                const auto& perm = leafPerms[leaf].perms[0];

                for (size_t i = 0; i < count; ++i) {
                    size_t globalIdx;
                    if constexpr (requires { octree.getLeafPoints(leaf); }) {
                        globalIdx = leafPointsLocal[perm[i]];
                    } else {
                        globalIdx = begin + perm[i];
                    }
                    const auto& p = points[globalIdx];
                    sortedFlat.allData[offset + i] = Point(globalIdx, p.getX(), p.getY(), p.getZ());
                }
            }
        }
        std::cout << "Finished building sorted data flat" << " with mode " << localReorderTypeToString(mode) << std::endl;
    }

    const SortedDataFlat& getSortedFlat() const {
        return sortedFlat;
    }

    // ============================================================
    // Función de construcción de permutaciones (NO reordena datos)
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
            // Variantes según el tipo de octree
            // Ptr expone un array de puntos
            // Linear expone un rango de índices en el array global de puntos
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
            // calcular claves
            // --------------------------------
            for (size_t i = 0; i < count; ++i)
            {
                // Obtener índice global del punto (i) en la hoja (linear: directo -- ptr: a través de leafPoints)
                
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

                // Ángulo phi en XY (0 a 2pi)
                if (mode == ReorderMode::Polar)
                {
                    const double phi = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
                    const double rxy = std::sqrt(dx * dx + dy * dy);
                    leafKeys[leaf].keys[0][i] = phi;
                }
                else if (mode == ReorderMode::Cartesian) // Coordenadas cartesianas: orden por X, Y, Z
                {
                    leafKeys[leaf].keys[0][i] = dx;
                    leafKeys[leaf].keys[1][i] = dy;
                    leafKeys[leaf].keys[2][i] = dz;
                }
            }

            // --------------------------------
            // ordenar permutaciones por claves KO, K1 y K2
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
                // Reordenar el vector de claves con el orden de perms
                
                for (size_t i = 0; i < count; ++i)
                    sortedK[i] = leafKeys[leaf].keys[k][perm[i]];
                leafKeys[leaf].keys[k] = sortedK;
            }
        }
        buildSortedDataFlat(octree, points, mode, leafPerms);
    }

    // ==========================
    // acceso a keys
    // ==========================
    const std::vector<double>& getLeafKeys(size_t leaf, OrderType type) const {
         return leafKeys[leaf].keys[static_cast<int>(type)];
    }

};