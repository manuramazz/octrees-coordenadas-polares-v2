# `/examples/` - Ejemplos de Uso de la Librería

Directorio que contiene ejemplos simples de cómo usar la librería del proyecto de forma directa desde código C++.

## Estructura

```
examples/
├── example.cpp         # Ejemplo simple y completo
└── CMakeLists.txt      # Configuración de compilación del ejemplo
```

---

## 📋 `example.cpp`
**Ejemplo completo de uso** de la librería del proyecto.

**Objetivo:**
Demostrar el flujo típico desde cargar datos hasta buscar vecinos.

### Contenido Aproximado

```cpp
#include <iostream>
#include <vector>

// Headers de la librería
#include "inc/geometry/point.hpp"
#include "inc/encoding/point_encoder_factory.hpp"
#include "inc/structures/linear_octree.hpp"
#include "inc/readers/file_reader_factory.hpp"

int main() {
    using Point = Point3f;
    using Container = std::vector<Point>;
    
    // 1. CARGAR DATOS
    std::cout << "1. Cargando datos...\n";
    auto reader = FileReaderFactory::create("data/paris_lille/Paris_Luxembourg_6.las");
    auto points = reader->read();
    std::cout << "   Cargados " << points.size() << " puntos\n";
    
    // 2. CODIFICAR PUNTOS (Morton 3D)
    std::cout << "2. Codificando con Morton 3D...\n";
    auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
    std::vector<uint64_t> codes(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        codes[i] = encoder->encode(
            static_cast<uint32_t>(points[i].x),
            static_cast<uint32_t>(points[i].y),
            static_cast<uint32_t>(points[i].z)
        );
    }
    
    // 3. ORDENAR PUNTOS POR CÓDIGO
    std::cout << "3. Ordenando puntos...\n";
    std::vector<size_t> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);  // [0, 1, 2, ...]
    std::sort(indices.begin(), indices.end(),
        [&codes](size_t a, size_t b) {
            return codes[a] < codes[b];
        });
    
    // Reordenar contenedor
    Container reordered(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        reordered[i] = points[indices[i]];
    }
    
    // 4. CONSTRUIR OCTREE
    std::cout << "4. Construyendo LinearOctree...\n";
    LinearOctree<Point> octree(reordered);
    
    // 5. BUSCAR VECINOS
    std::cout << "5. Buscando vecinos...\n";
    
    // 5a. Búsqueda por radio
    Point query = points[0];  // Primer punto como query
    float radius = 10.0f;
    
    auto neighbors_radius = octree.radius_search(query, radius);
    std::cout << "   Vecinos dentro de r=" << radius << ": " 
              << neighbors_radius.size() << "\n";
    
    for (size_t idx : neighbors_radius) {
        float dist = query.distance(points[idx]);
        std::cout << "      Punto " << idx << " a distancia " << dist << "\n";
    }
    
    // 5b. k-Nearest Neighbors
    int k = 5;
    auto neighbors_knn = octree.knn_search(query, k);
    std::cout << "   " << k << "-NN: " << neighbors_knn.size() << " vecinos\n";
    
    // 6. SALIDA
    std::cout << "\n✓ Ejemplo completado exitosamente\n";
    return 0;
}
```

---

## 🔧 `CMakeLists.txt`
Configuración para compilar el ejemplo.

**Contenido:**
```cmake
# El ejemplo se compila automáticamente con el proyecto principal
# Build:
#   cmake -B build .
#   cmake --build build
#
# Ejecutar:
#   ./build/examples/example
#
# Sin datos en datos/: descomentar y cambiar ruta

add_executable(example example.cpp)

# Linkear con librería principal
target_include_directories(example PRIVATE ${PROJECT_SOURCE_DIR}/inc)
target_link_libraries(example PRIVATE octrees_lib)

# Compilación
target_compile_features(example PRIVATE cxx_std_17)
target_compile_options(example PRIVATE -Wall -Wextra)

# Output
set_target_properties(example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)
```

---

## Pasos de Ejecución

### Compilar el ejemplo

```bash
# Desde raíz del proyecto
cmake -B build .
cmake --build build
```

Resultado: Ejecutable en `build/examples/example`

### Ejecutar

```bash
# Con datos disponibles
./build/examples/example

# Output esperado:
# 1. Cargando datos...
#    Cargados 1000000 puntos
# 2. Codificando con Morton 3D...
# 3. Ordenando puntos...
# 4. Construyendo LinearOctree...
# 5. Buscando vecinos...
#    Vecinos dentro de r=10.0: 234
#       Punto 0 a distancia 0
#       Punto 15 a distancia 3.2
#       Punto 123 a distancia 7.8
#    5-NN: 5 vecinos
# 
# ✓ Ejemplo completado exitosamente
```

---

## Conceptos Ilustrados

El ejemplo demuestra:

1. **Carga de datos** (LAS)
   ```cpp
   auto reader = FileReaderFactory::create(filename);
   auto points = reader->read();
   ```

2. **Codificación SFC** (Morton)
   ```cpp
   auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);
   uint64_t code = encoder->encode(x, y, z);
   ```

3. **Ordenamiento espacial** (preservar localidad)
   ```cpp
   std::sort(indices, [&codes](a, b) { return codes[a] < codes[b]; });
   ```

4. **Construcción de estructura** (LinearOctree)
   ```cpp
   LinearOctree octree(reordered);
   ```

5. **Búsquedas** (radio y kNN)
   ```cpp
   auto neighbors = octree.radius_search(query, radius);
   auto knn = octree.knn_search(query, k);
   ```

---

## Variaciones Comunes

### Usar diferentes estructuras

```cpp
// LinearOctree (estándar)
LinearOctree<Point> octree(reordered);

// Octree con reordenamiento polar
OctreeReordered<Point> octree(reordered, ReorderMode::Cylindrical);

// Octree tradicional con punteros
Octree<Point> octree(reordered);

// nanoflann KD-tree (para comparación)
auto kd_tree = std::make_unique<NanofannKDTree<Point>>(points);
```

### Usar diferentes codificadores

```cpp
// Morton 3D
auto encoder = PointEncoderFactory::create(EncoderType::MORTON_3D);

// Hilbert 3D
auto encoder = PointEncoderFactory::create(EncoderType::HILBERT_3D);

// Sin codificación (baseline)
auto encoder = PointEncoderFactory::create(EncoderType::NONE);
```

### Búsquedas múltiples

```cpp
// Múltiples puntos de query
for (size_t i = 0; i < 100; ++i) {
    Point query = points[rand() % points.size()];
    auto neighbors = octree.radius_search(query, 5.0f);
    std::cout << "Query " << i << ": " << neighbors.size() << " resultados\n";
}
```

### Con benchmarking

```cpp
// Medir tiempo de construcción
TimeWatcher watcher;
watcher.start();
LinearOctree octree(reordered);
watcher.stop();
std::cout << "Tiempo construcción: " 
          << watcher.elapsed_milliseconds() << " ms\n";

// Medir tiempo de búsqueda
watcher.reset();
watcher.start();
auto neighbors = octree.radius_search(query, radius);
watcher.stop();
std::cout << "Tiempo búsqueda: " 
          << watcher.elapsed_milliseconds() << " ms\n";
```

---

## Para Extender el Ejemplo

### Agregar nuevas funcionalidades

1. **Procesamiento de resultados**:
   ```cpp
   for (size_t idx : neighbors) {
       Point p = points[idx];
       float dist = query.distance(p);
       // Procesar cada vecino
   }
   ```

2. **Validación de resultados** (debugging):
   ```cpp
   // Verificar que todos los resultados son correctos
   for (size_t idx : neighbors_radius) {
       float dist = query.distance(points[idx]);
       assert(dist <= radius + 1e-6);  // Pequeña tolerancia
   }
   ```

3. **Exportar resultados**:
   ```cpp
   std::ofstream out("resultados.txt");
   for (size_t idx : neighbors) {
       out << idx << " " << query.distance(points[idx]) << "\n";
   }
   out.close();
   ```

---

## Notas

- El ejemplo es **simple e ilustrativo**
- Para casos reales, ver `src/main.cpp` para uso más complejo
- Requiere que `data/paris_lille/` exista con archivos .las
- Compilación estándar con `-O3` para mejor rendimiento

