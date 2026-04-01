# `/tests/` - Suite de Pruebas Unitarias

Directorio que contiene toda la suite de pruebas automatizadas del proyecto usando **GoogleTest** (framework de testing de C++).

## Estructura

```
tests/
├── CMakeLists.txt              # Configuración de tests
├── structures/                 # Tests de estructuras de datos
├── encoding/                   # Tests de codificadores
├── geometry/                   # Tests de geométrica
├── integration/                # Tests de integración
├── Testing/                    # Directorio generado por CTest
└── example.cpp                 # Ejemplo simple de uso
```

---

## Configuración de Build

### Compilar con tests
```bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build
```

### Ejecutar todos los tests
```bash
ctest --output-on-failure              # Con salida en caso de fallo
ctest --output-on-failure -V           # Verbose (más detalles)
ctest -R <pattern>                     # Ejecutar tests que match pattern
ctest -j 4                              # Ejecutar en paralelo (4 jobs)
```

### Ejecutar un test específico directamente
```bash
./tests/test_octree
./tests/test_encoders
./tests/test_geometry
```

---

## Subdirectorios

### 📁 `structures/`
Tests para todas las estructuras de búsqueda espacial.

**Archivos:**
- `test_octree.cpp` - Tests básicos de octree (construcción, búsqueda)
- `test_octree_advanced.cpp` - Tests avanzados (edge cases, robustez)
- `test_neighbor_set.cpp` - Tests del conjunto de vecinos

**Conjuntos de pruebas típicos:**
```cpp
// test_octree.cpp
TEST(OctreeTest, Construction) {
    // Verificar construcción correcta de árbol
}

TEST(OctreeTest, RadiusSearch) {
    // Verificar búsqueda por radio exacta
}

TEST(LinearOctreeTest, RadiusSearch) {
    // Comparar resultados con octree tradicional
}

TEST(OctreeReorderedTest, PolarCoordinates) {
    // Verificar reordenamiento en coordenadas polares
}
```

---

### 📁 `encoding/`
Tests para codificadores de curves que llenan el espacio.

**Archivos:**
- `test_encoders.cpp` - Tests de Morton y Hilbert

**Pruebas:**
- Codificación/decodificación correcta
- Preservación de proximidad
- Consistencia de orden
- Comparación Morton vs Hilbert

**Ejemplos:**
```cpp
TEST(MortonEncoderTest, Encode) {
    // Codificar puntos y verificar claves correctas
}

TEST(HilbertEncoderTest, Proximity) {
    // Verificar que puntos cercanos → claves cercanas
}

TEST(EncoderConsistency, DecodingIsInverse) {
    // Verificar que decode(encode(p)) ≈ p
}
```

---

### 📁 `geometry/`
Tests de estructuras geométricas.

**Archivos:**
- `test_points.cpp` - Tests de clase Point
- `test_geometry.cpp` - Tests de Box y operaciones

**Pruebas:**
- Operaciones de distancia (correctitud matemática)
- Contención de puntos en cajas
- Intersecciones esfera-caja
- Operaciones vectoriales (producto escalar, etc)

**Ejemplos:**
```cpp
TEST(PointTest, Distance) {
    Point p1(0, 0, 0);
    Point p2(3, 4, 0);
    EXPECT_FLOAT_EQ(p1.distance(p2), 5.0f);
}

TEST(BoxTest, Contains) {
    Box box({-1, -1, -1}, {1, 1, 1});
    EXPECT_TRUE(box.contains(Point(0, 0, 0)));
    EXPECT_FALSE(box.contains(Point(2, 0, 0)));
}

TEST(BoxTest, SphereIntersection) {
    Box box({0, 0, 0}, {1, 1, 1});
    Sphere sphere(Point(2, 0, 0), 2.0f);
    EXPECT_TRUE(box.intersects(sphere));
}
```

---

### 📁 `integration/`
Tests de integración que prueban flujos completos.

**Archivos:**
- `test_complex.cpp` - Flujos end-to-end

**Pruebas típicas:**
- Cargar datos → codificar → construir → buscar
- Comparación de resultados entre estructuras
- Validación de correctitud global
- Benchmarks mini

**Ejemplo:**
```cpp
TEST(IntegrationTest, FullPipeline) {
    // 1. Generar datos sintéticos
    auto points = generate_random_points(10000);
    
    // 2. Codificar
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    std::vector<uint64_t> codes = encode_all(points, encoder);
    
    // 3. Construir LinearOctree
    auto octree = LinearOctree(reorder(points, codes));
    
    // 4. Búsqueda y validación
    Point query = points[0];
    auto neighbors = octree.radius_search(query, 10.0f);
    
    // 5. Verificar que query está en sus propios resultados
    EXPECT_TRUE(neighbors.contains(0));
}
```

---

## Google Test Basics

### Estructura típica de un test

```cpp
#include <gtest/gtest.h>
#include "inc/structures/linear_octree.hpp"

// Test fixture (setup/teardown común)
class LinearOctreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Antes de cada test
        points = generate_test_points(1000);
    }
    
    void TearDown() override {
        // Después de cada test
        // (destrucción automática)
    }
    
    PointVector points;
};

// Test individual
TEST_F(LinearOctreeTest, Construction) {
    LinearOctree octree(points);
    EXPECT_EQ(octree.size(), 1000);
}

// Otro test sin fixture
TEST(MortonEncoderTest, SimpleEncoding) {
    MortonEncoder3D encoder;
    auto code = encoder.encode(1, 2, 3);
    EXPECT_NE(code, 0);
}
```

### Macros de aserción comunes

| Macro | Propósito |
|-------|-----------|
| `EXPECT_EQ(a, b)` | Verificar igualdad (no-fatal) |
| `ASSERT_EQ(a, b)` | Verificar igualdad (fatal, aborta test) |
| `EXPECT_FLOAT_EQ(a, b)` | Comparación de floats con tolerancia |
| `EXPECT_NEAR(a, b, eps)` | Valor dentro de épsilon |
| `EXPECT_TRUE(cond)` | Verificar condición verdadera |
| `EXPECT_FALSE(cond)` | Verificar condición falsa |
| `EXPECT_THROW(code, exc)` | Verificar excepción |
| `EXPECT_THAT(val, matcher)` | Matcher (gmock) |

---

## Ejecución desde CMake

### Script de compilación y test

```bash
#!/bin/bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build
ctest -j 4 --output-on-failure
```

O con código de retorno:
```bash
ctest --output-on-failure
RESULT=$?
if [ $RESULT -eq 0 ]; then
    echo "✓ Todos los tests pasaron"
else
    echo "✗ Algunos tests fallaron"
fi
exit $RESULT
```

---

## Cobertura de Tests

Ejecutar con cobertura de código:

```bash
cmake -B build -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage" .
cmake --build build
ctest
lcov --directory build --capture --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## Organización de Tests

### Niveles de Testing

1. **Unit Tests** (archivos en subdirectorios)
   - Pruebas de componentes individuales
   - Sin dependencias externas (mocks si es necesario)
   - Rápidos (< 1 segundo cada uno)

2. **Integration Tests** (`integration/`)
   - Flujos completos
   - Múltiples componentes juntos
   - Más lentos pero más realistas

3. **Performance Tests** (en benchmarking, no en tests/)
   - Medir tiempos
   - Comparar algoritmos
   - Generar gráficos

---

## Notas sobre Tests

### Buenas Prácticas
- ✅ Nombres descriptivos (TEST_Clase_Comportamiento)
- ✅ Un aspecto por test
- ✅ Datos pequeños (< 10K puntos) para rapidez
- ✅ Determinísticos (mismos inputs → mismos outputs)
- ✅ Independientes (no dependen de otros tests)

### Anti-patrones
- ❌ Tests interdependientes
- ❌ Tests lentos (> 1 seg) en unit tests
- ❌ Datos hardcoded sin significado
- ❌ No hacer assertions (tests que siempre pasan)

---

## Debugging Tests

### Ejecutar un test en gdb

```bash
gdb --args ./tests/test_octree --gtest_filter="OctreeTest.RadiusSearch"
```

### Verbose output
```bash
./tests/test_octree --gtest_print_time=1 --gtest_verbose
```

### Listar tests sin ejecutar
```bash
./tests/test_octree --gtest_list_tests
```

---

## CI/CD Integration

Típicamente en GitHub Actions o similar:

```yaml
- name: Run tests
  run: |
    cmake -B build -DBUILD_TESTS=ON .
    cmake --build build
    ctest --output-on-failure
```

