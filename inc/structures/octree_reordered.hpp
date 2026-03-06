#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <optional>


template<typename Octree_t, typename Container, typename key_t = typename PointEncoding::key_t>
class OctreeReordered {
public:

    static void swapPointsInPlace(PointsSoA& pts, size_t i, size_t j) {
        std::swap(pts.dataX()[i], pts.dataX()[j]);
        std::swap(pts.dataY()[i], pts.dataY()[j]);
        std::swap(pts.dataZ()[i], pts.dataZ()[j]);
        std::swap(pts.dataIds()[i], pts.dataIds()[j]);
    }

    /// TODO: openMP
    /// TODO: idea de evitar calcular atan2 para todos los puntos y usar orden angular sin trigonometría, lo que puede hacer el reordenamiento hasta 4–6× más rápido.
    /// TODO: tests
    static void reorderLeaves(
        Octree_t& octree,
        Container& points,
        std::vector<key_t>* codes = nullptr,
        std::optional<std::vector<PointMetadata>>* meta_opt = nullptr,
        ReorderMode mode = ReorderMode::None)
    {
        if (mode == ReorderMode::None)
            return;

        size_t numLeaves = octree.getNumLeaves();

        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            auto [begin, end] = octree.getLeafRange(leaf);
            size_t count = end - begin;

            if (count <= 1)
                continue;

            const auto& center = octree.getLeafCenter(leaf);

            std::vector<double> keys(count);
            std::vector<size_t> perm(count);

            // --------------------------------
            // calcular claves angulares
            // --------------------------------

            for (size_t i = 0; i < count; ++i)
            {
                size_t idx = begin + i;
                // Cálculo de dx, dy, dz dependiendo del tipo de contenedor
                // Para SoA es más eficiente acceder a los arrays directamente que mediante operador[] ??? CONSULTAR
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

                double key = 0.0;

                if (mode == ReorderMode::Cylindrical)
                {
                    key = std::atan2(dy, dx);
                    if (key < 0.0)
                        key += 2.0 * M_PI;
                }
                else if (mode == ReorderMode::Spherical)
                {
                    double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (r > 0.0)
                        key = std::acos(dz / r);
                }

                keys[i] = key;
                perm[i] = i;
            }

            // --------------------------------
            // ordenar permutación
            // --------------------------------

            std::sort(
                perm.begin(),
                perm.end(),
                [&](size_t a, size_t b)
                {
                    return keys[a] < keys[b];
                });

            // --------------------------------
            // aplicar permutación in-place
            // --------------------------------

            for (size_t i = 0; i < count; ++i)
            {
                size_t current = i;

                while (perm[current] != current)
                {
                    size_t next = perm[current];

                    if constexpr (std::is_same_v<Container, PointsSoA>) {
                        swapPointsInPlace(points, begin + current, begin + next);
                    } else {
                        std::swap(points[begin + current], points[begin + next]);
                    }

                    if (codes)
                        std::swap((*codes)[begin + current],
                                  (*codes)[begin + next]);

                    if (meta_opt && meta_opt->has_value())
                        std::swap((*meta_opt)->at(begin + current),
                                  (*meta_opt)->at(begin + next));

                    std::swap(perm[current], perm[next]);
                }
            }
        }
    }

    //Función reorderLeaves que implementa logs
    static void reorderLeavesLog(
        Octree_t& octree,
        Container& points,
        std::vector<key_t>* codes = nullptr,
        std::optional<std::vector<PointMetadata>>* meta_opt = nullptr,
        ReorderMode mode = ReorderMode::None)
    {
        if (mode == ReorderMode::None)
            return;

        size_t numLeaves = octree.getNumLeaves();
        std::cout << "Starting reordering of " << numLeaves << " leaves with mode "
                << (mode == ReorderMode::Cylindrical ? "Cylindrical" : "Spherical") << std::endl;
        for (size_t leaf = 0; leaf < numLeaves; ++leaf)
        {
            auto [begin, end] = octree.getLeafRange(leaf);
            size_t count = end - begin;
            if(leaf<100){
                std::cout << "Leaf " << leaf << " has " << count << " points." << std::endl; 
            }
            if (count <= 1)
                continue;

            const auto& center = octree.getLeafCenter(leaf);

            std::vector<double> keys(count);
            std::vector<size_t> perm(count);

            bool doLog = (leaf == 1 || leaf == 2); // solo hojas 1 y 2
            doLog = false;
            if (doLog) {
                std::cout << "Leaf " << leaf << " BEFORE reordering:\n";
                for (size_t i = begin; i < end; ++i) {
                    std::cout << "  idx " << i
                            << " -> x=" << points[i].getX()
                            << " y=" << points[i].getY()
                            << " z=" << points[i].getZ();
                    if (codes) std::cout << " code=" << (*codes)[i];
                    std::cout << "\n";
                }
            }

            // --------------------------------
            // calcular claves angulares
            // --------------------------------
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

                double key = 0.0;
                if (mode == ReorderMode::Cylindrical)
                {
                    key = std::atan2(dy, dx);
                    if (key < 0.0) key += 2.0 * M_PI;
                }
                else if (mode == ReorderMode::Spherical)
                {
                    double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (r > 0.0) key = std::acos(dz / r);
                }
                keys[i] = key;
                perm[i] = i;
            }

            // --------------------------------
            // ordenar permutación
            // --------------------------------
            std::sort(
                perm.begin(),
                perm.end(),
                [&](size_t a, size_t b) { return keys[a] < keys[b]; });

            // --------------------------------
            // aplicar permutación in-place
            // --------------------------------
            for (size_t i = 0; i < count; ++i)
            {
                size_t current = i;
                while (perm[current] != current)
                {
                    size_t next = perm[current];
                    if constexpr (std::is_same_v<Container, PointsSoA>) {
                        swapPointsInPlace(points, begin + current, begin + next);
                    } else {
                        std::swap(points[begin + current], points[begin + next]);
                    }

                    if (codes)
                        std::swap((*codes)[begin + current],
                                (*codes)[begin + next]);

                    if (meta_opt && meta_opt->has_value())
                        std::swap((*meta_opt)->at(begin + current),
                                (*meta_opt)->at(begin + next));
                    std::swap(perm[current], perm[next]);
                    
                }
            }

            if (doLog) {
                std::cout << "Leaf " << leaf << " AFTER reordering:\n";

                for (size_t i = begin; i < end; ++i)
                {
                    double dx, dy, dz;

                    if constexpr (std::is_same_v<Container, PointsSoA>) {
                        dx = points.dataX()[i] - center.getX();
                        dy = points.dataY()[i] - center.getY();
                        dz = points.dataZ()[i] - center.getZ();
                    } else {
                        dx = points[i].getX() - center.getX();
                        dy = points[i].getY() - center.getY();
                        dz = points[i].getZ() - center.getZ();
                    }

                    double key = 0.0;
                    const char* modeStr = (mode == ReorderMode::Cylindrical) ? "Cylindrical" : "Spherical";
                    if (mode == ReorderMode::Cylindrical) {
                        key = std::atan2(dy, dx);
                        if (key < 0.0) key += 2.0 * M_PI;
                    }
                    else if (mode == ReorderMode::Spherical) {
                        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                        if (r > 0.0) key = std::acos(dz / r);
                    }

                    std::cout << "  idx " << i
                            << " -> x=" << points[i].getX()
                            << " y=" << points[i].getY()
                            << " z=" << points[i].getZ()
                            << modeStr << "_key=" << key;

                    if (codes)
                        std::cout << " code=" << (*codes)[i];

                    std::cout << "\n";
                }

                std::cout << "----------------------------------\n";
            }
        }
    }
};