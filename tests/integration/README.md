# `/tests/integration/` - Tests de Integración

Directorio con pruebas de integración que verifican flujos completos del proyecto (cargar datos → codificar → construir → buscar).

## Archivos

```
integration/
└── test_complex.cpp    # Tests de flujos end-to-end
```

---

## 📋 `test_complex.cpp`
Tests que ejercen **múltiples componentes** juntos.

**Tests típicos:**

```cpp
// Fixture para tests complejos
class OctreeIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Datos reutilizables
        points = generate_random_points(10000);
    }
    
    std::vector<Point3f> points;
};

// Tests

TEST_F(OctreeIntegrationTest, FullPipeline) {
    // 1. Codificar
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    std::vector<uint64_t> codes(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        codes[i] = encoder->encode(points[i].x, points[i].y, points[i].z);
    }
    
    // 2. Ordenar
    std::vector<size_t> indices = argsort(codes);
    auto reordered = reorder(points, indices);
    
    // 3. Construir
    LinearOctree octree(reordered);
    
    // 4. Buscar
    Point3f query = points[100];
    auto neighbors = octree.radius_search(query, 5.0f);
    
    //Validar
    EXPECT_GT(neighbors.size(), 0);  // Al menos el punto mismo
    for (size_t idx : neighbors) {
        EXPECT_LE(query.distance(points[idx]), 5.0f + 1e-5);
    }
}

TEST_F(OctreeIntegrationTest, LinearVsPtr) {
    // Construir ambos octrees
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    // ... codificación y ordenamiento
    
    LinearOctree linear_octree(reordered);
    Octree ptr_octree(points);  // Sin reorden
    
    // Búsquedas idénticas
    Point3f query = points[0];
    float radius = 5.0f;
    
    auto linear_results = linear_octree.radius_search(query, radius);
    auto ptr_results = ptr_octree.radius_search(query, radius);
    
    // Ambos deben retornar MISMOS puntos
    EXPECT_EQ(linear_results.size(), ptr_results.size());
    
    for (size_t idx : linear_results) {
        EXPECT_TRUE(ptr_results.contains(idx));
    }
}

TEST_F(OctreeIntegrationTest, DifferentEncoders) {
    // Comparar Morton vs Hilbert
    auto morton = PointEncoderFactory::create(EncoderType::MORTON_3D);
    auto hilbert = PointEncoderFactory::create(EncoderType::HILBERT_3D);
    
    // Codificar y construir con ambos
    // ... (código omitido, similar pattern)
    
    LinearOctree morton_tree(morton_reordered);
    LinearOctree hilbert_tree(hilbert_reordered);
    
    // Búsquedas deben retornar MISMOS resultados
    // (aunque orden interno sea diferente)
    Point3f query = points[50];
    auto morton_results = morton_tree.radius_search(query, 10.0f);
    auto hilbert_results = hilbert_tree.radius_search(query, 10.0f);
    
    // Mismo conjunto de resultados
    EXPECT_EQ(morton_results.size(), hilbert_results.size());
    // Check cada índice está en ambos
}

TEST_F(OctreeIntegrationTest, KNNvsRadiusConsistency) {
    LinearOctree octree(reordered);
    Point3f query = points[100];
    
    // Buscar k=10
    auto knn_results = octree.knn_search(query, 10);
    EXPECT_EQ(knn_results.size(), 10);
    
    // El 10-ésimo vecino está a cierta distancia
    float max_dist = query.distance(points[knn_results.back()]);
    
    // Búsqueda por radio con ese radio debe incluir todos los k-NN
    auto radius_results = octree.radius_search(query, max_dist + 1e-5);
    
    // Todos los kNN deben estar en radius results
    for (size_t idx : knn_results) {
        EXPECT_TRUE(radius_results.contains(idx));
    }
}

TEST_F(OctreeIntegrationTest, PolarReordering) {
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    // ... codificación
    
    // SIN reordenamiento polar
    LinearOctree basic_octree(encoded_reordered);
    
    // CON reordenamiento polar
    OctreeReordered polar_octree(
        encoded_reordered,
        ReorderMode::Cylindrical
    );
    
    // Resultados deben ser idénticos
    Point3f query = points[0];
    auto basic_results = basic_octree.radius_search(query, 5.0f);
    auto polar_results = polar_octree.radius_search(query, 5.0f);
    
    // Mismo conjunto
    EXPECT_EQ(basic_results.size(), polar_results.size());
    for (size_t idx : basic_results) {
        EXPECT_TRUE(polar_results.contains(idx));
    }
}

TEST_F(OctreeIntegrationTest, RegressionTest) {
    // Verificación contra datos conocidos
    // Si tenemos ground truth
    
    LinearOctree octree(reordered);
    
    // Test contra resultado precalculado
    Point3f known_query(42.3, 17.8, 52.1);
    int expected_count = 127;  // Valor conocido
    
    auto results = octree.radius_search(known_query, 5.0f);
    EXPECT_EQ(results.size(), expected_count);
}

TEST_F(OctreeIntegrationTest, PerformanceAssertion) {
    LinearOctree octree(reordered);
    
    // Medir tiempo
    TimeWatcher timer;
    timer.start();
    
    for (int i = 0; i < 100; ++i) {
        Point3f query = points[rand() % points.size()];
        auto results = octree.radius_search(query, 5.0f);
    }
    
    timer.stop();
    double total_ms = timer.elapsed_milliseconds();
    double avg_time = total_ms / 100;
    
    // Verificación de performance
    // (No demasiado lento)
    EXPECT_LT(avg_time, 10.0);  // Menos de 10ms por búsqueda
}

TEST_F(OctreeIntegrationTest, StressTest) {
    LinearOctree octree(reordered);
    
    // Muchas búsquedas random
    for (int i = 0; i < 1000; ++i) {
        Point3f query = random_point_in_bounds();
        float radius = random_radius();
        
        auto results = octree.radius_search(query, radius);
        
        // Validación mínima
        EXPECT_GE(results.size(), 0);
        for (size_t idx : results) {
            EXPECT_LT(query.distance(points[idx]), radius + 1e-5);
        }
    }
}

TEST_F(OctreeIntegrationTest, MemorySanity) {
    // Crear octrees múltiples
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    // ...
    
    for (int i = 0; i < 10; ++i) {
        LinearOctree octree(reordered);
        // Sin leak de memoria (test de sanidad)
    }
    // Si hay leak, valgrind lo detectará
}
```

---

## Categorías de Tests

### 1. **Correctitud**: ¿Retorna resultados correctos?
```cpp
TEST_F(..., FullPipeline) { ... }
TEST_F(..., LinearVsPtr) { ... }
TEST_F(..., KNNvsRadiusConsistency) { ... }
```

### 2. **Equivalencia**: ¿Diferentes caminos retornan igual?
```cpp
TEST_F(..., DifferentEncoders) { ... }
TEST_F(..., PolarReordering) { ... }
```

### 3. **Performance**: ¿Es suficientemente rápido?
```cpp
TEST_F(..., PerformanceAssertion) { ... }
```

### 4. **Robustez**: ¿Maneja casos extremos?
```cpp
TEST_F(..., StressTest) { ... }
TEST_F(..., MemorySanity) { ... }
```

---

## Datos de Prueba

```cpp
std::vector<Point3f> generate_random_points(size_t count) {
    std::vector<Point3f> result;
    for (size_t i = 0; i < count; ++i) {
        result.push_back({
            random_float(-1000, 1000),
            random_float(-1000, 1000),
            random_float(0, 500)
        });
    }
    return result;
}
```

---

## Ejecución

```bash
./tests/test_complex
./tests/test_complex --gtest_filter="*Pipeline*"
./tests/test_complex --gtest_verbose
```

---

## Objetivos de Integration Tests

✓ Verificar que componentes funcionan **juntos**
✓ Detectar **regressions** en cambios
✓ Validar **invariantes** del sistema
✓ Medir **performance** real
✓ Prueba de **robustez** ante datos reales

