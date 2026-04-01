# `/inc/structures/` - Estructuras de Datos para Búsqueda Espacial

Módulo que implementa y adapta diversas estructuras de datos para búsqueda de vecinos en espacios 3D, con énfasis particular en octrees linealizados y reordenamiento en coordenadas polares.

## Archivos Principales

### 🌳 `octree.hpp`
Octree tradicional con **punteros explícitos** (árbol jerárquico).

**Características:**
- Cada nodo contiene 8 punteros a hijos o datos
- Estructura recursiva típica
- Mayor número de dereferencias de puntero
- Overhead de memoria por punteros

**Estructura:**
```
Nodo raíz
├─ Nodo hijo 0
│  ├─ Hoja 0 (puntos)
│  └─ Hoja 1 (puntos)
├─ Nodo hijo 1
└─ ...
```

**Ventajas:**
- Fácil de entender e implementar
- Estructura jerárquica natural
- Buena para problemas inciertos de densidad

**Desventajas:**
- Muchos accesos a memoria (punteros)
- Mala localidad de caché
- Overhead de almacenamiento

---

### ⚡ `linear_octree.hpp`
**Octree linealizado** - Versión mejorada sin punteros.

**Concepto clave:**
- Puntos reordenados por curva SFC (Morton/Hilbert)
- Estructura implícita sin punteros
- Orden de puntos codifica la jerarquía

**Ventajas:**
- ✅ Mejor localidad de caché
- ✅ Menos dereferencias de puntero
- ✅ Búsquedas más rápidas (~2-3x)
- ✅ Menor consumo de memoria

**Construcción:**
1. Codificar puntos (Morton/Hilbert)
2. Ordenar por código
3. Construir ranges de nodos implícitos
4. Acelerador de búsqueda

---

### 🌐 `octree_reordered.hpp`
**Octree Reordenado en Coordenadas Polares** - INNOVACIÓN PRINCIPAL del proyecto.

**Concepto:**
- Extiende `LinearOctree` con reordenamiento adicional
- Reordena puntos dentro de cada hoja usando coordenadas cilíndricas/esféricas
- Optimiza búsquedas por radio según orientación radial

**Modos de reordenamiento:**
1. **Cilíndrico**: (φ ángulo azimutal, r distancia, z altura)
2. **Esférico**: (φ ángulo azimutal, θ ángulo polar, r distancia)

**Ventaja:**
- Búsquedas por radio más eficientes
- Elimina más puntos en fases tempranas
- Mejor para datos distribuidos radialmente

**TODO (según comentarios del código):**
- [ ] Optimización: usar float en lugar de double
- [ ] Evitar sqrt innecesarios
- [ ] Precomputar (dx,dy) una sola vez por hoja
- [ ] Sin calcular acos para todos los puntos
- [ ] Verificar OpenMP funciona correctamente

---

### 👥 `neighbor_set.hpp`
Estructura para almacenar y gestionar el conjunto de vecinos encontrados.

**Funcionalidad:**
- Contenedor eficiente de índices de vecinos
- Soporte para búsquedas parciales
- Validación de resultados
- Comparación entre estructuras

**Métodos:**
- `add(index)` / `add(index, distance)`
- `size()`, `empty()`
- `contains(index)`
- Iteradores para recorrido

---

### 🔗 `nanoflann.hpp` y `nanoflann_wrappers.hpp`
Adaptadores para la librería **nanoflann** (KD-tree eficiente).

**nanoflann.hpp:**
- Copia de la librería nanoflann (header-only)
- Implementación de KD-tree optimizado

**nanoflann_wrappers.hpp:**
- Envuelve nanoflann en interfaz compatible del proyecto
- Permite comparación justa
- Benchmark de KD-tree vs. Octree

**Propósito:**
- Proporcionar punto de comparación estándar
- Evaluar ventajas del octree

---

### 🌲 `unibn_octree.hpp`
Adaptador para **unibnOctree** (otro octree de referencia).

**Características:**
- Envuelve octree de biblioteca externa
- Interfaz compatible para benchmarks
- Punto de comparación adicional

---

### 📦 `pcl_wrappers.hpp`
Adaptadores para **Point Cloud Library** (PCL).

**Contenido:**
- Wraps de `pcl::octree::OctreePointCloud`
- Wraps de `pcl::KDTree`
- Interfaz unificada para benchmarks
- Conversión de tipos de datos

**Propósito:**
- Comparación con librería estándar de PCL
- Validar rendimiento relativo
- Benchmark comprehensivo

---

### 🌳 `picotree_wrappers.hpp`
Adaptadores para **PicoTree** (estructura alternativa).

**Características:**
- Wrap de PicoTree
- Integración en pipeline de benchmarks

---

### 📊 `picotree_profiler.hpp`
Herramientas de profiling específico para PicoTree.

**Contenido:**
- Medición de memoria
- Análisis de estructura interna
- Comparativa de rendimiento

---

## Relación entre Estructuras

```
┌─────────────────────────────────────┐
│    Estructuras de Búsqueda Espacial │
└─────────────────────────────────────┘
          ↑              ↑              ↑
          │              │              │
    ┌─────┴─────┐   ┌────┴────┐   ┌────┴────┐
    │  Octrees   │   │ KD-trees│   │ Otros   │
    └─────┬─────┘   └────┬────┘   └────┬────┘
          │              │              │
    ┌─────┴──────────┐  │         ┌────┴─────┐
    │                │  │         │           │
┌───┴────┐   ┌──────┴──┴──┐  ┌───┴───┐   ┌─┴──────┐
│Ptr-Tree│   │Linear+Polar│  │nanofl. │  │PicoTree│
└────────┘   │(Principal) │  │ PCL    │  └────────┘
             └────────────┘  └────────┘
```

---

## Comparación de Estructuras

| Aspecto | Ptr-Tree | Linear | Polar | nanoflann | PCL | PicoTree |
|---------|----------|--------|-------|-----------|-----|----------|
| **Velocidad búsqueda** | ⚡⚡ | ⚡⚡⚡ | ⚡⚡⚡⚡ | ⚡⚡⚡ | ⚡⚡ | ⚡⚡⚡ |
| **Memoria** | 😞 | ✓ | ✓ | ✓ | 😞 | ✓ |
| **Cache-friendly** | ✗ | ✓ | ✓✓ | ✓ | ✗ | ✓ |
| **Construcción** | ⚡ | ⚡ | ⚡⚡ | ⚡⚡ | ⚡ | - |

---

## Flujo de Uso Típico

### 1. Construcción

```cpp
// Cargar puntos
auto points = reader->read_file(filename);

// Codificar (crear orden SFC)
auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
std::vector<uint64_t> codes(points.size());
for (size_t i = 0; i < points.size(); ++i) {
    codes[i] = encoder->encode(points[i].x, points[i].y, points[i].z);
}

// Ordenar por código
std::vector<size_t> indices = sort_by_code(codes);
auto reordered = reorder(points, indices);

// Construir estructura
LinearOctree octree(reordered);
// O para coordenadas polares:
OctreeReordered octree(reordered, ReorderMode::Cylindrical);
```

### 2. Búsqueda de Vecinos

```cpp
// Buscar k vecinos más cercanos
NeighborSet neighbors = octree.knn_search(query_point, k);

// O búsqueda por radio
NeighborSet neighbors = octree.radius_search(query_point, radius);
```

### 3. Benchmarking

```cpp
auto benchmark = std::make_unique<NeighborBenchmark>(octree);
auto results = benchmark->run();
results.save_to_json("results.json");
```

---

## Notas de Implementación

- Todas las estructuras heredan de interfaz común (virtual)
- Búsquedas usan recursión con early termination
- OpenMP para paralelización de búsquedas masivas
- Métodos inline para operaciones críticas

---

## Testing

Ver `tests/structures/` para:
- `test_octree.cpp` - Pruebas de octree básico
- `test_octree_advanced.cpp` - Pruebas de funcionalidad avanzada
- `test_polar_reordering.cpp` - Tests específicos del reordenamiento polar
- `test_neighbor_set.cpp` - Tests del conjunto de vecinos

