# `/tests/structures/` - Tests de Estructuras de Datos

Directorio con pruebas unitarias para todas las estructuras de búsqueda espacial del proyecto (octrees, KD-trees, wrappers, etc).

## Archivos

```
structures/
├── test_octree.cpp                  # Tests básicos del octree
├── test_octree_advanced.cpp        # Tests avanzados y edge cases
└── test_neighbor_set.cpp           # Tests del conjunto de vecinos
```

---

## 📋 `test_octree.cpp`
Tests **básicos de construcción y búsqueda** de octrees.

**Conjuntos de pruebas (TEST_F o TEST):**

### LinearOctreeTest
```cpp
TEST_F(LinearOctreeTest, Construction) {
    // Verificar que octree se construye correctamente
}

TEST_F(LinearOctreeTest, RadiusSearch) {
    // Verificar búsqueda por radio
    // - Todos los puntos retornados están dentro del radio
    // - No faltan puntos que deberían estar
}

TEST_F(LinearOctreeTest, KNNSearch) {
    // Verificar búsqueda k-NN
    // - Se retornan exactamente k puntos
    // - Son los k más cercanos
    // - Distancias en orden creciente
}

TEST_F(LinearOctreeTest, EmptySearchResults) {
    // Búsqueda con radio muy pequeño
    // - Puede retornar pocos/ningún punto
}

TEST_F(LinearOctreeTest, LargeRadius) {
    // Búsqueda con radio muy grande
    // - Retorna muchos o todos los puntos
}
```

### OctreeTest (Octree tradicional)
```cpp
TEST_F(OctreeTest, Construction) { /*...*/ }
TEST_F(OctreeTest, RadiusSearch) { /*...*/ }
TEST_F(OctreeTest, KNNSearch) { /*...*/ }
```

### ResultValidation
```cpp
TEST(ResultValidation, LinearOctreeEqualsOctree) {
    // Verificar que resultados de LinearOctree == Octree tradicional
    // La estructura es diferente pero resultados deben ser iguales
}

TEST(ResultValidation, KNNExactResults) {
    // Verificar exactitud de k-NN vs fuerza bruta
}
```

---

## 📋 `test_octree_advanced.cpp`
Tests **avanzados, edge cases y propiedades geométricas**.

**Pruebas típicas:**

```cpp
TEST(OctreeAdvanced, ConsistencyAcrossDimensions) {
    // Búsqueda 2D vs 3D debe ser coherente
}

TEST(OctreeAdvanced, DuplicatePoints) {
    // Manejo de puntos duplicados (misma posición)
}

TEST(OctreeAdvanced, CollinearPoints) {
    // Puntos en una línea recta
}

TEST(OctreeAdvanced, VerySparseData) {
    // Puntos muy dispersos, pocos vecinos
}

TEST(OctreeAdvanced, VeryDenseData) {
    // Muchos puntos en volumen pequeño
}

TEST(OctreeAdvanced, SearchAtBoundary) {
    // Búsqueda en límites del espacio
}

TEST(OctreeReorderedAdvanced, PolarConsistency) {
    // Reordenamiento polar preserva resultados
}

TEST(OctreeReorderedAdvanced, CylindricalVsSpherical) {
    // Comparar reordenamientos cilíndrico vs esférico
}

TEST(PerformanceComparison, LinearVsPtr) {
    // LinearOctree debe ser más rápido que Octree con punteros
}

TEST(MemoryComparison, LinearVsPtr) {
    // LinearOctree usa menos memoria que Octree
}
```

---

## 📋 `test_neighbor_set.cpp`
Tests de la estructura `NeighborSet`.

**Pruebas:**

```cpp
TEST(NeighborSetTest, Add) {
    NeighborSet set;
    set.add(5);
    EXPECT_TRUE(set.contains(5));
    EXPECT_EQ(set.size(), 1);
}

TEST(NeighborSetTest, Uniqueness) {
    // Agregar índice duplicado
    set.add(5);
    set.add(5);
    // Debe existir solo una vez
    EXPECT_EQ(set.size(), 1);
}

TEST(NeighborSetTest, Ordering) {
    // Verificar orden de distancias (para kNN)
}

TEST(NeighborSetTest, Iteration) {
    // Iterar sobre elementos
    for (size_t idx : neighbors) {
        // Uso típico
    }
}

TEST(NeighborSetTest, Comparison) {
    // Comparar dos NeighborSets
}
```

---

## Estructura de Tests Típica

```cpp
#include <gtest/gtest.h>
#include "inc/structures/linear_octree.hpp"
#include "inc/encoding/point_encoder_factory.hpp"

// Fixture: setup común para múltiples tests
class LinearOctreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generar datos de prueba reutilizables
        points = generate_random_points(10000);
        
        // Crear octree
        auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
        codes = encode_all(points, encoder);
        indices = argsort(codes);
        reordered = reorder(points, indices);
        
        octree = std::make_unique<LinearOctree<Point3f>>(reordered);
    }
    
    std::vector<Point3f> points;
    std::vector<uint64_t> codes;
    std::unique_ptr<LinearOctree<Point3f>> octree;
};

// Test individual dentro del fixture
TEST_F(LinearOctreeTest, RadiusSearch) {
    Point3f query = points[0];
    float radius = 5.0f;
    
    auto neighbors = octree->radius_search(query, radius);
    
    // Validar resultados
    for (size_t idx : neighbors) {
        float dist = query.distance(points[idx]);
        EXPECT_LE(dist, radius + 1e-5f);  // Tolerancia numérica
    }
}
```

---

## Ejecución

### Compilar solo tests de structures

```bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build

# Ejecutar todos
ctest -R ".*Test" --output-on-failure

# O directamente
./tests/test_octree
./tests/test_octree_advanced
./tests/test_neighbor_set
```

### Con filtro específico

```bash
./tests/test_octree --gtest_filter="LinearOctreeTest.RadiusSearch"
```

---

## Validaciones Típicas

### Búsqueda por Radio

✓ Todos los puntos retornados cumplen: `dist(p, query) <= radius`
✓ No hay puntos válidos omitidos
✓ Número de resultados razonable

### Búsqueda k-NN

✓ Retorna exactamente k puntos (o menos si hay < k totales)
✓ Son los k más cercanos (distancia mínima)
✓ Distancias en orden ascendente

### Consistencia

✓ LinearOctree retorna mismos resultados que Octree
✓ Resultados independientes del método de construcción
✓ Búsquedas múltiples idénticas retornan resultados iguales

---

## Datos de Prueba

Los tests generan datos sintéticos:

```cpp
std::vector<Point3f> generate_random_points(size_t count) {
    std::vector<Point3f> points;
    for (size_t i = 0; i < count; ++i) {
        points.push_back({
            random_float(-100, 100),
            random_float(-100, 100),
            random_float(0, 100)
        });
    }
    return points;
}
```

Casos especiales probados:
- Pequeñas cantidades (10-100 puntos)
- Medianas (1000-10000 puntos)
- Grandes (100000+ puntos)

---

## Notas

- Tests son **determinísticos** (seeds fijos)
- Pueden ejecutarse **en paralelo** (sin state compartido)
- **Rápidos**: segundos para toda suite
- Cobertura de **edge cases** importante

