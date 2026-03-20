#pragma once

#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <omp.h>


template<typename Octree_t, typename Container>
class OctreeReordered {
public:

    /*Tengo que hacer 3 arrays de punteros -> angulo phi, angulo theta, r para esféricas y ángulo phi, r, z?? para cilíndricas
    hago las tres reordenaciones en la misma función para cada hoja

    Luego en el proceso de búsqueda -> si se elimina o se incluye por completo no hago nada -> si se incluye parcialmente -> 
    tengo que hacer los cálculos para ver que clave elimina más volumen de la hoja -> uso esa reordenación para eliminar/incluir puntos de la hoja.
    */
    /// TODO: mirar que funcione bien openMP
    /*TODO: OPTIMIZACIONES PARA LUEGO: usar float en vez de double
    evitar sqrt
    precomputar (dx,dy) una sola vez por hoja
    no calcular acos para todos los puntos*/

    enum class OrderType {
        K0 = 0,
        K1 = 1,
        K2 = 2
    };

    struct LeafPermutations {
        std::array<std::vector<size_t>, 3> perms;
    };

    std::vector<LeafPermutations> leafPerms;


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

        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            auto [begin, end] = octree.getLeafRange(leaf);
            size_t count = end - begin;

            if (count <= 1)
                continue;

            const auto& center = octree.getLeafCenter(leaf);

            // claves por dimensión
            std::array<std::vector<double>, 2> keys;
            for (int k = 0; k < 2; ++k)
                keys[k].resize(count);

            // inicializar permutaciones
            for (int k = 0; k < 3; ++k) {
                leafPerms[leaf].perms[k].resize(count);
                std::iota(leafPerms[leaf].perms[k].begin(),
                          leafPerms[leaf].perms[k].end(), 0);
            }

            // --------------------------------
            // calcular claves
            // --------------------------------
            std::vector<double> dxVals(count);
            std::vector<double> dyVals(count);
            std::vector<double> rxyVals(count);

            for (size_t i = 0; i < count; ++i)
            {
                size_t idx = begin + i;

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

                const double rxy = std::sqrt(dx * dx + dy * dy);
                dxVals[i] = dx;
                dyVals[i] = dy;
                rxyVals[i] = rxy;

                if (mode == ReorderMode::Cylindrical)
                {
                    keys[0][i] = rxy;
                    keys[1][i] = dz;
                }
                else // Spherical
                {
                    const double r = std::sqrt(rxy * rxy + dz * dz);
                    double theta = (r > 0.0) ? std::acos(std::clamp(dz / r, -1.0, 1.0)) : 0.0;
                    keys[0][i] = theta;
                    keys[1][i] = r;
                }
            }

            // K0: orden angular en XY sin atan2 usando semiplanos y producto cruzado.
            auto angleLess = [&](size_t i, size_t j) {
                if (i == j)
                    return false;

                const double x1 = dxVals[i];
                const double y1 = dyVals[i];
                const double x2 = dxVals[j];
                const double y2 = dyVals[j];

                const bool origin1 = (x1 == 0.0 && y1 == 0.0);
                const bool origin2 = (x2 == 0.0 && y2 == 0.0);

                if (origin1 != origin2)
                    return origin1;

                if (origin1)
                    return i < j;

                const bool upper1 = (y1 > 0.0) || (y1 == 0.0 && x1 >= 0.0);
                const bool upper2 = (y2 > 0.0) || (y2 == 0.0 && x2 >= 0.0);

                if (upper1 != upper2)
                    return upper1;

                const double cross = x1 * y2 - y1 * x2;
                if (cross != 0.0)
                    return cross > 0.0;

                if (rxyVals[i] != rxyVals[j])
                    return rxyVals[i] < rxyVals[j];

                return i < j;
            };

            auto& permK0 = leafPerms[leaf].perms[static_cast<int>(OrderType::K0)];
            std::sort(permK0.begin(), permK0.end(), angleLess);

            // --------------------------------
            // ordenar cada permutación
            // --------------------------------
            for (int k = 0; k < 2; ++k)
            {
                auto& perm = leafPerms[leaf].perms[k+1];

                std::sort(
                    perm.begin(),
                    perm.end(),
                    [&](size_t a, size_t b) {
                        return keys[k][a] < keys[k][b];
                    });
            }
        }
    }

    // ==========================
    // acceso a permutación
    // ==========================

    const std::vector<size_t>& getLeafPermutation(size_t leaf, OrderType type) const {
        return leafPerms[leaf].perms[static_cast<int>(type)];
    }

    // ============================================================
    // Variante de debug para verificar OpenMP y permutaciones
    // ============================================================

    void buildLeafPermutationsDebug(
        Octree_t& octree,
        Container& points,
        ReorderMode mode,
        bool printPreview = true)
    {
        auto debugLog = [](const std::string& msg) {
            #pragma omp critical(octree_reordered_debug_log)
            {
                std::cout << msg << '\n';
            }
        };

        auto previewPermutation = [&](const std::vector<size_t>& perm, size_t maxItems) {
            std::ostringstream oss;
            const size_t limit = std::min(maxItems, perm.size());
            for (size_t i = 0; i < limit; ++i) {
                if (i > 0)
                    oss << ',';
                oss << perm[i];
            }
            if (perm.size() > limit)
                oss << ",...";
            return oss.str();
        };

        auto isValidPermutation = [](const std::vector<size_t>& perm, size_t expectedSize) {
            if (perm.size() != expectedSize)
                return false;

            std::vector<size_t> sortedPerm = perm;
            std::sort(sortedPerm.begin(), sortedPerm.end());

            for (size_t i = 0; i < expectedSize; ++i) {
                if (sortedPerm[i] != i)
                    return false;
            }
            return true;
        };

        if (mode == ReorderMode::None) {
            debugLog("[OctreeReordered::Debug] mode=None, no se construyen permutaciones.");
            return;
        }

        const size_t numLeaves = octree.getNumLeaves();
        leafPerms.resize(numLeaves);

        {
            std::ostringstream oss;
            oss << "[OctreeReordered::Debug] start leaves=" << numLeaves
                << " max_threads=" << omp_get_max_threads();
            debugLog(oss.str());
        }

        #pragma omp parallel for schedule(dynamic)
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            const int tid = omp_get_thread_num();
            auto [begin, end] = octree.getLeafRange(leaf);
            const size_t count = end - begin;
            const bool logLeaf = (leaf < 10);

            if (logLeaf) {
                std::ostringstream oss;
                oss << "[OctreeReordered::Debug] leaf=" << leaf
                    << " tid=" << tid
                    << " range=[" << begin << ',' << end << ")"
                    << " count=" << count;
                debugLog(oss.str());
            }

            if (count <= 1) {
                if (logLeaf) {
                    std::ostringstream oss;
                    oss << "[OctreeReordered::Debug] leaf=" << leaf
                        << " tid=" << tid
                        << " skipped (count<=1)";
                    debugLog(oss.str());
                }
                continue;
            }

            const auto& center = octree.getLeafCenter(leaf);

            std::array<std::vector<double>, 2> keys;
            for (int k = 0; k < 2; ++k)
                keys[k].resize(count);

            for (int k = 0; k < 3; ++k) {
                leafPerms[leaf].perms[k].resize(count);
                std::iota(leafPerms[leaf].perms[k].begin(),
                          leafPerms[leaf].perms[k].end(), 0);
            }

            std::vector<double> dxVals(count);
            std::vector<double> dyVals(count);
            std::vector<double> rxyVals(count);

            for (size_t i = 0; i < count; ++i)
            {
                const size_t idx = begin + i;

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

                const double rxy = std::sqrt(dx * dx + dy * dy);
                dxVals[i] = dx;
                dyVals[i] = dy;
                rxyVals[i] = rxy;

                if (mode == ReorderMode::Cylindrical)
                {
                    keys[0][i] = rxy;
                    keys[1][i] = dz;
                }
                else
                {
                    const double r = std::sqrt(rxy * rxy + dz * dz);
                    const double theta = (r > 0.0) ? std::acos(std::clamp(dz / r, -1.0, 1.0)) : 0.0;
                    keys[0][i] = theta;
                    keys[1][i] = r;
                }
            }

            auto angleLess = [&](size_t i, size_t j) {
                if (i == j)
                    return false;

                const double x1 = dxVals[i];
                const double y1 = dyVals[i];
                const double x2 = dxVals[j];
                const double y2 = dyVals[j];

                const bool origin1 = (x1 == 0.0 && y1 == 0.0);
                const bool origin2 = (x2 == 0.0 && y2 == 0.0);

                if (origin1 != origin2)
                    return origin1;

                if (origin1)
                    return i < j;

                const bool upper1 = (y1 > 0.0) || (y1 == 0.0 && x1 >= 0.0);
                const bool upper2 = (y2 > 0.0) || (y2 == 0.0 && x2 >= 0.0);

                if (upper1 != upper2)
                    return upper1;

                const double cross = x1 * y2 - y1 * x2;
                if (cross != 0.0)
                    return cross > 0.0;

                if (rxyVals[i] != rxyVals[j])
                    return rxyVals[i] < rxyVals[j];

                return i < j;
            };

            auto& permK0 = leafPerms[leaf].perms[static_cast<int>(OrderType::K0)];
            std::sort(permK0.begin(), permK0.end(), angleLess);

            if (logLeaf) {
                const bool k0Sorted = std::is_sorted(permK0.begin(), permK0.end(), angleLess);
                const bool k0Valid = isValidPermutation(permK0, count);
                std::ostringstream oss;
                oss << "[OctreeReordered::Debug] leaf=" << leaf
                    << " tid=" << tid
                    << " K0 sorted=" << (k0Sorted ? "OK" : "FAIL")
                    << " valid_perm=" << (k0Valid ? "OK" : "FAIL");
                if (printPreview)
                    oss << " preview=" << previewPermutation(permK0, 10);
                debugLog(oss.str());
            }

            for (int k = 0; k < 2; ++k)
            {
                auto& perm = leafPerms[leaf].perms[k + 1];
                std::sort(
                    perm.begin(),
                    perm.end(),
                    [&](size_t a, size_t b) {
                        return keys[k][a] < keys[k][b];
                    });

                if (logLeaf) {
                    const bool sorted = std::is_sorted(
                        perm.begin(),
                        perm.end(),
                        [&](size_t a, size_t b) {
                            return keys[k][a] < keys[k][b];
                        });
                    const bool valid = isValidPermutation(perm, count);

                    std::ostringstream oss;
                    oss << "[OctreeReordered::Debug] leaf=" << leaf
                        << " tid=" << tid
                        << " K" << (k + 1)
                        << " sorted=" << (sorted ? "OK" : "FAIL")
                        << " valid_perm=" << (valid ? "OK" : "FAIL");
                    if (printPreview)
                        oss << " preview=" << previewPermutation(perm, 10);
                    debugLog(oss.str());
                }
            }
        }

        debugLog("[OctreeReordered::Debug] finished");
    }
};