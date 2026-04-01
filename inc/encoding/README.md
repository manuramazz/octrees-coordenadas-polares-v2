# `/inc/encoding/` - Codificadores de Curvas que Llenan el Espacio

Módulo que implementa diferentes esquemas de codificación de puntos 3D en códigos 1D usando curvas que llenan el espacio (Space Filling Curves - SFC). Estas curvas preservan la proximidad espacial, mejorando la localidad de caché.

## Concepto General

Las curvas que llenan el espacio mapean puntos en un espacio 3D a códigos enteros en una línea 1D, manteniendo la proximidad:
- Puntos cercanos en 3D → Códigos cercanos en 1D
- Mejor localidad de caché → Mejor rendimiento de búsqueda

## Archivos Principales

### 🔧 `point_encoder.hpp`
Clase base abstracta `PointEncoder` que define la interfaz para todos los codificadores.

**Métodos principales:**
- `encode(x, y, z)` - Codifica un punto individual en clave (uint64)
- `encodeVectorized(x[], y[], z[], keys[])` - Codifica múltiples puntos (SIMD)
- `decode(code, x, y, z)` - Decodifica una clave a coordenadas
- `maxDepth()` - Profundidad máxima del codificador
- `eps()` - Épsilon de precisión
- `upperBound()` - Clave máxima posible

**Tipos:**
- `coords_t = uint_fast32_t` - Coordenadas discretizadas
- `key_t = uint_fast64_t` - Código de curva SFC

---

### 🏭 `point_encoder_factory.hpp`
Factory para crear instancias de codificadores.

**Responsabilidades:**
- Instanciar el codificador correcto según tipo (Morton/Hilbert)
- Proporcionar una única instancia por tipo
- Valida parámetros de entrada

**Uso típico:**
```cpp
auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
auto code = encoder->encode(x, y, z);
```

---

### 🟦 `morton_encoder_2d.hpp`
Implementación de la curva de Morton para 2D.

**Características:**
- Codificación entrelazada de bits X e Y
- Complejidad O(1)
- Fórmula rápida sin bucles

**Patrón:**
```
Punto (x, y) → Interleave bits → Código Morton
   101        010             10010101
   101        101             
```

---

### 🟦 `morton_encoder_3d.hpp`
Extensión de Morton a 3D.

**Características:**
- Entrelazado de X, Y, Z
- Mantiene la proximidad 3D
- Rápida y simple
- Base para octrees lineales

**Ventajas:**
- Implementación muy rápida
- Buen balance de localidad
- Compatible con estructura de octree

---

### 🟩 `hilbert_encoder_2d.hpp`
Implementación de la curva de Hilbert para 2D.

**Características:**
- Curva más compleja que Morton
- Mejor preservación de proximidad local
- Cálculo más lento que Morton
- Requiere LUT (Look-Up Tables)

**Ventajas sobre Morton:**
- Mejor localidad de caché
- Menos "saltos" en el espacio
- Mejor para análisis de clustering

---

### 🟩 `hilbert_encoder_3d.hpp`
Extensión de Hilbert a 3D.

**Características:**
- Preservación de proximidad superior a Morton 3D
- LUT predefinidas para cada orientación
- Cálculo iterativo de curva
- Mejor rendimiento en algunos benchmarks

**Nota:** Hilbert 3D es más complejo que Morton 3D pero puede dar mejores resultados de localidad.

---

### 🚫 `no_encoding.hpp`
Codificador "dummy" sin reordenamiento real.

**Propósito:**
- Punto de comparación (baseline)
- Medir rendimiento sin SFC
- Control de experimentos

**Comportamiento:**
- Devuelve índices originales como claves
- Sin transformación de coordenadas

---

### 📚 `libmorton/`
Directorio que contiene la biblioteca externa `libmorton`.

**Contenido:**
- Implementaciones optimizadas de Morton en diferentes plataformas
- Soporte para SIMD (SSE4, AVX)
- Implementación fallback en CPU pura

**Ventajas:**
- Altamente optimizado
- Soporte multiplataforma
- Funciones vectorizadas

---

## Tipos y Definiciones

```cpp
using coords_t = uint_fast32_t;   // Coordenadas discretizadas (0-2^32)
using key_t = uint_fast64_t;      // Código SFC resultante (0-2^64)
```

## Proceso de Codificación

1. **Normalización**: Puntosoriginales [min, max] → Coordenadas discretas [0, 2^32)
2. **Codificación**: Aplicar SFC (Morton o Hilbert) → Claves [0, 2^64)
3. **Reordenamiento**: Puntos ordenados por clave
4. **Construcción**: Usar orden para construcción de estructura

---

## Comparación de Codificadores

| Aspecto | Morton | Hilbert | None |
|---------|--------|---------|------|
| **Velocidad** | ⚡⚡⚡ | ⚡⚡ | ⚡⚡⚡ |
| **Localidad** | ✓✓ | ✓✓✓ | ✗ |
| **Dimesniones** | 2D, 3D | 2D, 3D | - |
| **Complejidad** | Simple | Compleja | - |
| **Cache behavior** | Bueno | Muy bueno | Malo |

---

## Uso en el Proyecto

Los codificadores se utilizan:
1. En fase de preprocesamiento para reordenar puntos
2. Durante construcción de `LinearOctree` para determinar posiciones
3. En `OctreeReordered` como base para reordenamiento posterior
4. Comparación de localidad en benchmarks (`bench_locality.bash`)

---

## Optimizaciones

### Vectorización (SIMD)
- Método `encodeVectorized` procesa múltiples puntos simultáneamente
- Usa SSE4 o AVX según disponibilidad
- Speedup significativo para lotes grandes

### Look-Up Tables (LUT)
- Hilbert usa LUT para orientaciones
- Evita cálculos repetitivos
- Trade-off: memoria por velocidad

### Inlining
- Métodos `encode` son inline para O(1) amortizado
- Compilador puede optimizar agresivamente

---

## Testing

Ver `tests/encoding/test_encoders.cpp` para:
- Verificación de codificación/decodificación
- Tests de localidad
- Comparación de códigos entre puntos similares

