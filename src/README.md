# `/src/` - Implementación del Proyecto

Directorio que contiene todos los archivos de implementación (.cpp) del proyecto. Estos archivos implementan las definiciones de header-only y contienen código que requiere compilación.

## Estructura

```
src/
├── main.cpp              # Punto de entrada principal
├── main_options.cpp     # Parseo de opciones de línea de comandos
├── octree.cpp           # Implementación del octree tradicional
├── time_watcher.cpp     # Implementación del cronómetro
├── papi_events.cpp      # Integración con PAPI
└── (la mayoría son headers, ver inc/)
```

## Archivos Principales

### 🚀 `main.cpp`
**Punto de entrada del programa** - `int main(int argc, char* argv[])`

**Responsabilidades:**
1. Parsea argumentos de línea de comandos
2. Valida opciones del usuario
3. Carga archivo de datos (LAS)
4. Codifica puntos (Morton/Hilbert)
5. Construye estructura de búsqueda (Octree/KD-tree/etc)
6. Ejecuta benchmarks requeridos
7. Exporta resultados

**Flujo:**
```cpp
int main() {
    // 1. Parsear args
    auto opts = MainOptions::parse(argc, argv);
    
    // 2. Validar
    if (!opts.is_valid()) {
        print_usage();
        return 1;
    }
    
    // 3. Leer datos
    auto reader = FileReaderFactory::create(opts.input_file);
    auto points = reader->read();
    
    // 4. Codificar
    auto encoder = PointEncoderFactory::create(opts.encoder_type);
    auto codes = encode_all(points, encoder);
    
    // 5. Construir estructura
    auto structure = build_structure(points, codes, opts.structure_type);
    
    // 6. Benchmarks
    run_benchmarks(structure, points, opts);
    
    // 7. Salida
    save_results(opts.output_file);
    return 0;
}
```

**Opciones soportadas:**
- `-i, --input` - Archivo LAS de entrada (obligatorio)
- `-o, --output` - Archivo de salida para resultados
- `-r, --radii` - Radii para búsqueda por radio (ej: 2.5,5.0,10.0)
- `-v, --kvalues` - Valores de k para kNN (ej: 10,50,100,1000)
- `-c, --container-type` - AoS o SoA para almacenamiento
- `-s, --structure` - Estructura (LinearOctree, Octree, KDTree, PCL, etc)
- `-a, --algorithm` - Algoritmo de búsqueda
- `-e, --encoder` - Tipo de codificador (Morton, Hilbert, None)
- `-m, --reorder-mode` - Modo de reordenamiento (None, Cylindrical, Spherical)
- `--bench-type` - Tipo de benchmark a ejecutar
- `--num-queries` - Número de puntos de búsqueda aleatorios
- `-h, --help` - Mostrar ayuda

---

### ⚙️ `main_options.cpp`
Implementación del parseo de opciones de línea de comandos.

**Contenido:**
- `MainOptions::parse()` - Parsear argumentos con `getopt`
- `MainOptions::is_valid()` - Validar opciones consistentes
- `MainOptions::print_usage()` - Mostrar ayuda
- Mapeos entre strings y enumeraciones

**Validaciones:**
```cpp
bool is_valid() {
    // Archivo existe
    if (!std::filesystem::exists(input_file)) return false;
    
    // Valores de radii positivos
    for (auto r : search_radii) {
        if (r <= 0) return false;
    }
    
    // Valores de k positivos
    for (auto k : k_values) {
        if (k <= 0) return false;
    }
    
    // Archivo salida writable
    // ... más validaciones
    
    return true;
}
```

---

### 🌳 `octree.cpp`
Implementación del **octree tradicional** con punteros explícitos.

**Contenido:**
- Construcción de árbol jerárquico
- Manejo de nodos y punteros
- Métodos de búsqueda recursivos
- Memory management (new/delete)

**Interfaz principal:**
```cpp
class Octree {
public:
    Octree(const PointContainer& points);  // Constructor
    ~Octree();                              // Destructor (liberar memoria)
    
    NeighborSet radius_search(Point q, float r);
    NeighborSet knn_search(Point q, int k);
    
private:
    struct Node {
        Box bbox;
        std::vector<size_t> points;  // Indices en contenedor original
        std::array<Node*, 8> children;  // Punteros a hijos
    };
    
    Node* root;
};
```

**Algoritmo de construcción:**
```
1. Si #puntos <= max_per_node:
   - Crear hoja con puntos
   - Retornar
2. Si no:
   - Dividir bbox en 8 octantes
   - Asignar puntos a octantes
   - Recursivamente construir hijos
```

---

### ⏱️ `time_watcher.cpp`
Implementación del **cronómetro de precisión**.

**Características:**
- Resolución de nanosegundos
- Múltiples puntos de medición
- Estadísticas de tiempos

**Interfaz:**
```cpp
class TimeWatcher {
public:
    void start();                         // Iniciar medición
    void stop();                          // Detener medición
    
    double elapsed_seconds() const;
    double elapsed_milliseconds() const;
    double elapsed_nanoseconds() const;
    
    void reset();                         // Reiniciar
    
    // Para múltiples mediciones
    void add_checkpoint(std::string name);  // Guardar checkpoint
    double time_since(std::string name);    // Tiempo desde checkpoint
};
```

**Uso típico:**
```cpp
TimeWatcher watcher;
watcher.add_checkpoint("start");

// Operación 1
perform_task_1();
std::cout << "Task 1: " << watcher.time_since("start") << " ms\n";

// Operación 2
watcher.add_checkpoint("task2_start");
perform_task_2();
std::cout << "Task 2: " << watcher.time_since("task2_start") << " ms\n";
```

---

### 🔍 `papi_events.cpp`
Integración con **PAPI** (Performance API) para profiling.

**Contenido:**
- Inicialización de contador PAPI
- Recolección de eventos
- Cálculo de métricas (cache hits/misses, etc)

**Eventos medidos (selección):**
- `PAPI_L1_DCM` - Level 1 cache misses (datos)
- `PAPI_L2_DCM` - Level 2 cache misses
- `PAPI_LL_DCM` - Last Level cache misses
- `PAPI_BR_MSP` - Branch mispredictions
- `PAPI_FP_OPS` - Operaciones flotantes
- `PAPI_TOT_CYC` - Total ciclos

**Interfaz:**
```cpp
class PAPIProfiler {
public:
    PAPIProfiler();
    ~PAPIProfiler();
    
    void start_measurement(const std::vector<int>& events);
    void stop_measurement();
    
    std::vector<long long> get_values();
    double get_cache_miss_rate();
    double get_ipc();  // Instructions per cycle
};
```

---

## Proceso de Compilación

El CMake compila estos archivos junto con los headers:

```bash
# Compilación Release optimizada
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build

# Resultado: build/octrees-benchmark (ejecutable)
```

## Relación Típica

```
inc/*(headers) +
src/*.cpp      +  → CMake → compilador C++ → build/octrees-benchmark
CMakeLists.txt
```

---

## Depuración

### Símbolos de depuración
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug .
cmake --build build
gdb ./build/octrees-benchmark
```

### Profiling
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo .
cmake --build build

# Con perf
perf record ./build/octrees-benchmark ...
perf report

# Con PAPI
./build/octrees-benchmark --papi ...
```

---

## Testing

Los tests se compilan separadamente:
```bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build
ctest --output-on-failure
```

Ver `/tests/` para code de tests.

---

## Notas

- Los headers (`.hpp`) contienen templates y se compilan en cada traducción
- Los sources (`.cpp`) evitan recompilación innecesaria con templates fijos
- OpenMP usado para paralelización
- C++17 o superior requerido

