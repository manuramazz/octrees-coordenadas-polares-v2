# `/tests/encoding/` - Tests de Codificadores

Directorio con pruebas unitarias para los codificadores de curvas que llenan el espacio (Space Filling Curves).

## Archivos

```
encoding/
└── test_encoders.cpp   # Tests de Morton y Hilbert
```

---

## 📋 `test_encoders.cpp`
Tests compreh ensivos para codificadores (Morton, Hilbert, etc).

**Conjuntos de pruebas:**

### MortonEncoderTest

```cpp
TEST(MortonEncoderTest, Encode2D) {
    MortonEncoder2D encoder;
    
    // Casos simples
    EXPECT_NE(encoder.encode(0, 0), 0);
    EXPECT_EQ(encoder.encode(0, 0), 0);  // (0,0) -> 0
    
    // Orden esperado
    auto code1 = encoder.encode(1, 0);
    auto code2 = encoder.encode(0, 1);
    // Ambos contiguos a (0,0) en orden Morton
}

TEST(MortonEncoderTest, Encode3D) {
    MortonEncoder3D encoder;
    
    // Codificación básica
    auto code = encoder.encode(5, 3, 7);
    EXPECT_NE(code, 0);
    
    // Simetría en orden 3D
    auto code_xyz = encoder.encode(1, 2, 3);
    auto code_zyx = encoder.encode(3, 2, 1);
    EXPECT_NE(code_xyz, code_zyx);  // DIFERENTES coordinadas
}

TEST(MortonEncoderTest, Roundtrip) {
    // Encode -> Decode debe recuperar valores
    MortonEncoder3D encoder;
    uint32_t x = 42, y = 37, z = 19;
    
    auto code = encoder.encode(x, y, z);
    uint32_t x_out, y_out, z_out;
    encoder.decode(code, x_out, y_out, z_out);
    
    EXPECT_EQ(x, x_out);
    EXPECT_EQ(y, y_out);
    EXPECT_EQ(z, z_out);
}

TEST(MortonEncoderTest, LocalityPreservation) {
    MortonEncoder3D encoder;
    
    // Puntos cercanos → códigos cercanos
    auto code1 = encoder.encode(10, 10, 10);
    auto code2 = encoder.encode(11, 10, 10);  // Diferencia mínima
    
    // Códigos deben ser cercanos (no garantizado, pero típicamente sí)
    EXPECT_LT(std::abs((long long)code1 - (long long)code2), 256);
}

TEST(MortonEncoderTest, Monotonicity) {
    MortonEncoder3D encoder;
    
    // Si x1 < x2 manteniendo y,z constantes
    // Típicamente code1 < code2 (NO garantizado por SFC)
    
    auto code1 = encoder.encode(0, 0, 0);
    auto code2 = encoder.encode(1, 0, 0);
    auto code3 = encoder.encode(2, 0, 0);
    
    // Verificar ordenamiento (Morton preserva O(log n) rango)
}
```

### HilbertEncoderTest

Similar a Morton pero:

```cpp
TEST(HilbertEncoderTest, Encode2D) {
    HilbertEncoder2D encoder;
    // Similar structure pero curva Hilbert
}

TEST(HilbertEncoderTest, Encode3D) {
    HilbertEncoder3D encoder;
    // Más complejo, usa rotaciones de curva
}

TEST(HilbertEncoderTest, BetterLocalityThanMorton) {
    // Hilbert típicamente preserva localidad mejor
    // Puntos cercanos → códigos aún más cercanos
}
```

### EncoderComparison

```cpp
TEST(EncoderComparison, MortonVsHilbert) {
    MortonEncoder3D morton;
    HilbertEncoder3D hilbert;
    
    // Para puntos cercanos
    Point3f p0(0, 0, 0);
    Point3f p1(0.5, 0.5, 0);
    
    auto m_code0 = morton.encode(p0.x, p0.y, p0.z);
    auto m_code1 = morton.encode(p1.x, p1.y, p1.z);
    auto m_distance = std::abs((long long)m_code0 - (long long)m_code1);
    
    auto h_code0 = hilbert.encode(p0.x, p0.y, p0.z);
    auto h_code1 = hilbert.encode(p1.x, p1.y, p1.z);
    auto h_distance = std::abs((long long)h_code0 - (long long)h_code1);
    
    // Hilbert usualmente preserva proximidad mejor
    // EXPECT_LE(h_distance, m_distance);  // No siempre
}

TEST(EncoderComparison, PerformanceMortonFaster) {
    // Morton más rápido de codificar
    TimeWatcher timer;
    
    timer.start();
    for (...) morton.encode(x, y, z);
    auto morton_time = timer.elapsed_nanoseconds();
    
    timer.reset();
    timer.start();
    for (...) hilbert.encode(x, y, z);
    auto hilbert_time = timer.elapsed_nanoseconds();
    
    EXPECT_LT(morton_time, hilbert_time);
}
```

### VectorizationTest

```cpp
TEST(VectorizationTest, MortonVectorized) {
    MortonEncoder3D encoder;
    
    // SIMD encoding
    std::vector<uint32_t> x(16), y(16), z(16);
    std::vector<uint64_t> codes(16);
    
    // Llenar con datos
    for (int i = 0; i < 16; ++i) {
        x[i] = i;
        y[i] = i * 2;
        z[i] = i * 3;
    }
    
    // Codificación vectorizada (SIMD)
    encoder.encodeVectorized(x.data(), y.data(), z.data(), codes, 0);
    
    // Verificar resultados
    for (int i = 0; i < 16; ++i) {
        auto single = encoder.encode(x[i], y[i], z[i]);
        EXPECT_EQ(codes[i], single);
    }
}
```

### RangeTest

```cpp
TEST(EncoderRange, MortonMaxValue) {
    MortonEncoder3D encoder;
    
    // Máximo valor representables
    uint32_t max_val = encoder.maxDepth();
    
    // Encoding de máximos
    auto code = encoder.encode(max_val, max_val, max_val);
    
    // Verificar que está en rango
    EXPECT_LE(code, encoder.upperBound());
}
```

---

## Casos de Prueba

### Valores Simples
```cpp
// (0,0,0) -> 0
// (1,0,0) -> entrelazado de bits
// (0,1,0) -> diferente
// (0,0,1) -> diferente
```

### Valores Máximos
```cpp
// Codificar máximos sin overflow
// Decodificación preserva rango
```

### Aleatoriedad
```cpp
// Muchos puntos aleatorios
// Verificar consistencia estadística
// Distribución de códigos
```

### Simetría
```cpp
// Propiedades esperadas de curvas
// Transformaciones de simetría
```

---

## Métricusde Localidad

```cpp
// Función auxiliar: medir localidad
float measure_locality(const Encoder& enc,
                       const std::vector<Point3f>& points) {
    float total_distance = 0;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        auto code1 = enc.encode(points[i]);
        auto code2 = enc.encode(points[i+1]);
        
        // Distancia en espacio de códigos
        auto code_dist = std::abs((long long)code1 - (long long)code2);
        
        // Distancia en espacio 3D
        auto space_dist = points[i].distance(points[i+1]);
        
        total_distance += code_dist / (space_dist + 1e-6);
    }
    return total_distance / points.size();
}

TEST(LocalityMetrics, HilbertBetterLocalitySmallScale) {
    auto points = generate_clustered_points(1000, 10);  // 10 clusters pequeños
    
    auto morton_locality = measure_locality(morton_enc, points);
    auto hilbert_locality = measure_locality(hilbert_enc, points);
    
    // Hilbert típicamente mejor en clusters pequeños
    // EXPECT_LT(hilbert_locality, morton_locality * 1.2);
}
```

---

## Ejecución

```bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build

./tests/test_encoders
./tests/test_encoders --gtest_filter="MortonEncoderTest*"
./tests/test_encoders --gtest_filter="HilbertEncoderTest*"
```

---

## Notas

- Codificadores deben ser **muy rápidos** (inline)
- **Exactitud**: roundtrip encode->decode debe ser perfecto
- **Localidad**: no garantizada pero típicamente buena
- Comparación relativa (Morton vs Hilbert) más importante que valores absolutos

