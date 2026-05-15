// octree_types.hpp
#pragma once

enum class OrderType {
    K0 = 0, // orden angular en XY sin atan2 usando semiplanos y producto cruzado
    K1 = 1, // orden por clave 1 (rxy para cilíndricas, theta para esféricas)
    K2 = 2  // orden por clave 2 (z para cilíndricas, r para esféricas)
};


struct SortedDataFlat {
    bool isPolar = false;
    std::vector<Point> allData; 
    std::vector<size_t> leafOffsets; // leafOffsets[leaf] = inicio en allData

    /**
     * Devuelve el puntero al inicio del bloque de puntos de una hoja para un eje dado.
     */
    [[nodiscard]] inline const Point* leafData(size_t leaf, size_t count, int axis) const {
        const int mult = isPolar ? 1 : 3; 
        return allData.data() + (leafOffsets[leaf] * mult) + (axis * count);
    }

    /**
     * Permite acceder a un punto específico por referencia.
     */
    [[nodiscard]] inline const Point& getPoint(size_t leaf, size_t count, int axis, size_t localIdx) const {
        return leafData(leaf, count, axis)[localIdx];
    }

    [[nodiscard]] inline size_t leafCount(size_t leaf) const {
        return leafOffsets[leaf + 1] - leafOffsets[leaf];
    }
};


struct PrunedRange {
    size_t iMin  = 0;
    size_t iMax  = 0;
    size_t iMin2 = 0; 
    size_t iMax2 = 0;
    bool hasSecond = false;
    OrderType order = OrderType::K0;

    [[nodiscard]] inline size_t count() const { return hasSecond ? (iMax - iMin) + (iMax2 - iMin2) : (iMax - iMin); }
};


// Clase de utilidades matemáticas
namespace detail {

    inline constexpr double kPi    = 3.14159265358979323846;
    inline constexpr double kTwoPi = 2.0 * kPi;

    inline double effectiveXYRadius(double radius, Kernel_t kernel) {
        if (kernel == Kernel_t::cube) {
            return radius * std::sqrt(2.0);
        }if (kernel == Kernel_t::sphere) {
            return radius;
        }if (kernel == Kernel_t::square) {
            return radius * std::sqrt(2.0);
        }if (kernel == Kernel_t::circle) {
            return radius;
        }
        return radius;
    }


    struct LeafQueryGeometry {
        double dx, dy, dz;
        double dxy;
        double radius=0, rEff=0, rxyEff=0;

        static LeafQueryGeometry computePolar(
            const Point& query,
            const Point& center,
            double radius,
            Kernel_t kernel)
        {
            LeafQueryGeometry g;
            g.dx  = query.getX() - center.getX();
            g.dy  = query.getY() - center.getY();
            g.dz  = query.getZ() - center.getZ();
            g.dxy = std::sqrt(g.dx * g.dx + g.dy * g.dy);
            g.rxyEff = effectiveXYRadius(radius, kernel);
            return g;
        }
        static LeafQueryGeometry computeCart(
            const Point& query,
            const Point& center,
            double radius,
            Kernel_t kernel)
        {
            LeafQueryGeometry g;
            g.dx  = query.getX() - center.getX();
            g.dy  = query.getY() - center.getY();
            g.dz  = query.getZ() - center.getZ();
            g.radius = radius;
            return g;
        }
    };


    inline double normalizeAngle0To2Pi(double a) {
        double out = std::fmod(a, kTwoPi);
        return out < 0.0 ? out + kTwoPi : out;
    }

    inline bool kernelContainsLeafPerAxis(const LeafQueryGeometry& geo, const Vector& leafRadii, OrderType order) {
        const double eps = 1e-9;
        if (order == OrderType::K0) {
            return (geo.dx - geo.radius < - leafRadii.getX() - eps) && (geo.dx + geo.radius > leafRadii.getX() + eps);
        } else if (order == OrderType::K1) {
            return (geo.dy - geo.radius < - leafRadii.getY() - eps) && (geo.dy + geo.radius > leafRadii.getY() + eps);
        } else {
            return (geo.dz - geo.radius < - leafRadii.getZ() - eps) && (geo.dz + geo.radius > leafRadii.getZ() + eps);
        }
        return false;
    
    }



} // namespace detail
