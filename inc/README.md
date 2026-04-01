# `/inc/` - Headers y Librerías del Proyecto

Directorio que contiene todos los archivos de encabezado (.hpp) que definen las estructuras de datos, algoritmos y utilidades principales del proyecto.

## Estructura

```
inc/
├── benchmarking/      # Módulos de medición de rendimiento
├── encoding/          # Codificadores de curvas que llenan el espacio
├── structures/        # Estructuras de búsqueda espacial
├── geometry/          # Primitivas geométricas
├── kernels/           # Kernels de búsqueda
├── readers/           # Lectores de archivos de datos
├── main_options.hpp   # Opciones y enumeraciones de línea de comandos
└── util.hpp           # Utilidades generales
```

## Subdirectorios Principales

### 📁 `benchmarking/`
Contiene toda la infraestructura para medir y registrar el rendimiento de las distintas estructuras y algoritmos.

**Archivos principales:**
- `benchmarking.hpp` - Interfaz base para benchmarks
- `neighbor_benchmarks.hpp` - Benchmarks de búsqueda de vecinos (kNN y radio)
- `enc_build_benchmarks.hpp` - Benchmarks de construcción y codificación
- `memory_benchmarks.hpp` - Medición de memoria utilizada
- `locality_benchmarks.hpp` - Análisis de localidad de caché
- `papi_events.hpp` - Integración con PAPI para profiling de caché
- `encoding_log.hpp` - Registro de información de codificación
- `build_log.hpp` - Registro de información de construcción
- `search_set.hpp` - Conjuntos de puntos de búsqueda
- `time_watcher.hpp` - Cronómetro de precisión

**Propósito:** Proporcionar herramientas para medir tiempos de ejecución, uso de memoria, accesos a caché y otras métricas de rendimiento durante la construcción de estructuras y búsquedas.

---

### 📁 `encoding/`
Implementaciones de diferentes esquemas de codificación de puntos 3D usando curvas que llenan el espacio.

**Archivos principales:**
- `point_encoder.hpp` - Clase base abstracta para codificadores
- `point_encoder_factory.hpp` - Factory para crear codificadores
- `morton_encoder_2d.hpp` - Codificador Morton 2D
- `morton_encoder_3d.hpp` - Codificador Morton 3D
- `hilbert_encoder_2d.hpp` - Codificador Hilbert 2D
- `hilbert_encoder_3d.hpp` - Codificador Hilbert 3D
- `no_encoding.hpp` - Codificador sin reordenamiento
- `libmorton/` - Biblioteca externa para cálculos de Morton

**Propósito:** Convertir puntos 3D en códigos que preservan la localidad espacial. Las curvas que llenan el espacio (Morton y Hilbert) mapean el espacio 3D a una línea 1D manteniendo la proximidad, mejorando la localidad de caché.

---

### 📁 `structures/`
Implementaciones de diferentes estructuras de datos para búsqueda espacial.

**Archivos principales:**
- `octree.hpp` - Octree tradicional con punteros
- `linear_octree.hpp` - Octree linealizado (sin punteros)
- `octree_reordered.hpp` - **Octree con reordenamiento en coordenadas polares** (cilíndricas/esféricas)
- `neighbor_set.hpp` - Conjunto de vecinos encontrados
- `nanoflann.hpp`, `nanoflann_wrappers.hpp` - Wraps de nanoflann KD-tree
- `pcl_wrappers.hpp` - Wraps para PCL Octree y KD-tree
- `picotree_wrappers.hpp` - Wraps para PicoTree
- `picotree_profiler.hpp` - Profiler para PicoTree
- `unibn_octree.hpp` - Wrap para unibnOctree

**Propósito:** Proporcionar distintas estructuras de búsqueda espacial para comparación. El foco principal es la optimización mediante linearización y reordenamiento en coordenadas polares.

---

### 📁 `geometry/`
Definiciones de primitivas geométricas básicas.

**Archivos principales:**
- `point.hpp` - Clase Point para representar puntos 3D
- `point_containers.hpp` - Contenedores de puntos (Array of Structures y Structure of Arrays)
- `point_metadata.hpp` - Metadatos asociados a puntos
- `box.hpp` - Caja 3D para representar nodos de octree

**Propósito:** Proporcionar tipos de datos geométricos eficientes para almacenar y manipular puntos y cajas de búsqueda.

---

### 📁 `kernels/`
Implementaciones de kernels de búsqueda para diferentes dimensiones y tipos de consultas.

**Archivos principales:**
- `kernel_abstract.hpp` - Clase base para kernels
- `kernel_2d.hpp` - Kernel de búsqueda 2D
- `kernel_3d.hpp` - Kernel de búsqueda 3D
- `kernel_factory.hpp` - Factory para crear kernels

**Propósito:** Encapsular la lógica de búsqueda por radio y validación de puntos para búsqueda de vecinos, permitiendo especializaciones por dimensión.

---

### 📁 `readers/`
Lectores de archivos de datos (formato LAS - LiDAR).

**Archivos principales:**
- `file_reader.hpp` - Clase base para lectores
- `las_file_reader.hpp` - Lector secuencial de archivos LAS
- `las_file_reader_parallel.hpp` - Lector paralelo de archivos LAS (OpenMP)
- `file_reader_factory.hpp` - Factory para crear lectores
- `handlers.hpp` - Manejadores de puntos

**Propósito:** Proporcionar interfaces para leer eficientemente datos LiDAR desde archivos LAS, tanto de forma secuencial como paralela.

---

## Archivos de Nivel Superior

### `main_options.hpp`
Define todas las opciones y enumeraciones para la línea de comandos:
- `SearchStructure` - Enumeración de estructuras disponibles (Octree, KD-tree, etc.)
- `SearchAlgo` - Enumeración de algoritmos de búsqueda
- `ReorderMode` - Modos de reordenamiento (Ninguno, Cilíndrico, Esférico)
- Maps de enumeraciones a strings para parseo de argumentos

### `util.hpp`
Funciones utilitarias generales del proyecto.

---

## Relaciones entre Módulos

```
main.cpp
  ↓
readers/ (cargar puntos LAS)
  ↓
encoding/ (codificar con Morton/Hilbert)
  ↓
structures/ (construir octree)
  ├── geometry/ (usar Point, Box)
  └── benchmarking/ (medir rendimiento)
  ↓
kernels/ (búsqueda de vecinos)
  ↓
benchmarking/ (registrar resultados)
```

---

## Notas de Desarrollo

- Todos los archivos son templates header-only (.hpp)
- Se usa C++17 y posterior
- OpenMP para paralelización
- PAPI opcional para profiling detallado de caché

