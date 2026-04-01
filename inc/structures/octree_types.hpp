// octree_types.hpp
#pragma once

enum class OrderType {
    K0 = 0, // orden angular en XY sin atan2 usando semiplanos y producto cruzado
    K1 = 1, // orden por clave 1 (rxy para cilíndricas, theta para esféricas)
    K2 = 2  // orden por clave 2 (z para cilíndricas, r para esféricas)
};


struct PrunedRange {
    size_t iMin  = 0;
    size_t iMax  = 0;
    OrderType order = OrderType::K0;
    [[nodiscard]] size_t count() const { return iMax - iMin; }
};