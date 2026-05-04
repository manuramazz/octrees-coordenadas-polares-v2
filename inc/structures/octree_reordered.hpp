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


template<typename Octree_t, typename Container>
class OctreeReordered {
public:

    /*Tengo que hacer 3 arrays de punteros -> angulo phi, angulo theta, r para esféricas y ángulo phi, r, z?? para cilíndricas
    hago las tres reordenaciones en la misma función para cada hoja

    Luego en el proceso de búsqueda -> si se elimina o se incluye por completo no hago nada -> si se incluye parcialmente -> 
    tengo que hacer los cálculos para ver que clave elimina más volumen de la hoja -> uso esa reordenación para eliminar/incluir puntos de la hoja.
    */
    /// TODO: 
    // optimizar la función de cálculo de claves para cada punto (precomputar dx, dy, dz, rxy, etc)
    /*TODO: OPTIMIZACIONES PARA LUEGO: usar float en vez de double
    evitar sqrt
    no calcular acos para todos los puntos*/

    struct LeafPermutations {
        std::array<std::vector<size_t>, 3> perms;
    };
    struct LeafKeys {
        std::array<std::vector<double>, 3> keys;
    };

    std::vector<LeafPermutations> leafPerms;
    std::vector<LeafKeys> leafKeys;

    /**
    ** REORDENACIÓN FÍSICA (duplicación del vector de puntos)
    **/

    std::vector<LeafSortedData> leafSortedData;

    // ============================================================
    // Función de construcción de vectores reordenados (duplicación)
    // ============================================================
    void buildSortedData(
        Octree_t& octree,
        Container& points,
        OrderType order,
        ReorderMode mode)
    {
        if (mode == ReorderMode::None)
            return;

        const size_t numLeaves = octree.getNumLeaves();
        leafSortedData.resize(numLeaves);

        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            const auto& perm = leafPerms[leaf].perms[static_cast<int>(order)];
            const size_t count = perm.size();

            if (count == 0) continue;

            leafSortedData[leaf].points.resize(count);
            leafSortedData[leaf].globalIdxs.resize(count);

            size_t begin = 0;
            std::vector<size_t> leafPointsLocal;

            if constexpr (requires { octree.getLeafPoints(leaf); }) {
                leafPointsLocal = octree.getLeafPoints(leaf);
            } else {
                auto [b, e] = octree.getLeafRange(leaf);
                begin = b;
            }

            for (size_t i = 0; i < count; ++i) {
                size_t globalIdx;
                if constexpr (requires { octree.getLeafPoints(leaf); }) {
                    globalIdx = leafPointsLocal[perm[i]];
                } else {
                    globalIdx = begin + perm[i];
                }

                leafSortedData[leaf].globalIdxs[i] = globalIdx;

                if constexpr (std::is_same_v<Container, PointsSoA>) {
                    leafSortedData[leaf].points[i] = Point(
                        points.dataX()[globalIdx],
                        points.dataY()[globalIdx],
                        points.dataZ()[globalIdx]);
                } else {
                    leafSortedData[leaf].points[i] = points[globalIdx];
                }
            }
        }
    }

    const LeafSortedData& getSortedLeafData(size_t leaf) const {
        return leafSortedData[leaf];
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
            if (count == 0)
                continue;
            
            if(count == 1) {
                // Si solo hay un punto, las tres permutaciones son iguales (la única posición)
                for (int k = 0; k < 3; ++k) {
                    leafPerms[leaf].perms[k].resize(1);
                    leafPerms[leaf].perms[k][0] = 0;
                }
                continue;
            }

            const auto& center = octree.getLeafCenter(leaf);

            // inicializar permutaciones y claves
            for (int k = 0; k < 3; ++k) {
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

                const double phi = detail::normalizeAngle0To2Pi(std::atan2(dy, dx));
                const double rxy = std::sqrt(dx * dx + dy * dy);

                if (mode == ReorderMode::Cylindrical)
                {
                    leafKeys[leaf].keys[0][i] = phi;
                    leafKeys[leaf].keys[1][i] = rxy;
                    leafKeys[leaf].keys[2][i] = dz;
                }
                else // Spherical
                {
                    const double r = std::sqrt(rxy * rxy + dz * dz);
                    double theta = (r > 0.0) ? std::acos(std::clamp(dz / r, -1.0, 1.0)) : 0.0;
                    leafKeys[leaf].keys[0][i] = phi;
                    leafKeys[leaf].keys[1][i] = theta;
                    leafKeys[leaf].keys[2][i] = r;
                }
            }

            // --------------------------------
            // ordenar permutaciones por claves KO, K1 y K2
            // --------------------------------
            std::vector<double> sortedK(count);
            for (int k = 0; k < 3; ++k)
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
    }

    // ==========================
    // acceso a permutación
    // ==========================

    const std::vector<size_t>& getLeafPermutation(size_t leaf, OrderType type) const {
         return leafPerms[leaf].perms[static_cast<int>(type)];
    }

    const std::vector<double>& getLeafKeys(size_t leaf, OrderType type) const {
         return leafKeys[leaf].keys[static_cast<int>(type)];
    }

};