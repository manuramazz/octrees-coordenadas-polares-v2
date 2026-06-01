// octree_types.hpp
#pragma once

enum class OrderType {
    K0 = 0, // 
    K1 = 1, // 
    K2 = 2  //
};


struct SortedDataFlat {
    std::vector<Point> PointsPolar;
    std::vector<Point> PointsX;
    std::vector<Point> PointsY;
    std::vector<Point> PointsZ;
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

struct RangeDebugTimers {
    double projectionTimeSec = 0.0;
    double binarySearchTimeSec = 0.0;
};


// Clase de utilidades matemáticas
namespace detail {

    inline constexpr double kPi    = 3.14159265358979323846;
    inline constexpr double kTwoPi = 2.0 * kPi;

    inline double effectiveXYRadius(double radius, Kernel_t kernel) {
        if (kernel == Kernel_t::cube) {
            return radius * 1.41421356237309;
        }if (kernel == Kernel_t::sphere) {
            return radius;
        }if (kernel == Kernel_t::square) {
            return radius * 1.41421356237309;
        }if (kernel == Kernel_t::circle) {
            return radius;
        }
        return radius;
    }


    struct LeafQueryGeometry {
        double dx, dy, dz;
        double dxy;
        double radius=0, rEff=0, rxyEff=0;
    };


    inline double normalizeAngle0To2Pi(double a) {
        double out = std::fmod(a, kTwoPi);
        return out < 0.0 ? out + kTwoPi : out;
    }



} // namespace detail
