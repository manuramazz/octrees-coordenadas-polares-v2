# `/inc/readers/` - Lectores de Archivos de Datos

Módulo que proporciona interfaces para leer eficientemente archivos de datos LAS (formato estándar LiDAR) en memoria.

## Concepto

Los **lectores** abstraen la complejidad de leer archivos LAS:
- Parseo del formato binario
- Conversión de tipos
- Manejo de distintos layouts
- Lectura secuencial y paralela

## Archivos Principales

### 📄 `file_reader.hpp`
Interfaz base abstracta `FileReader<PointType>`.

**Métodos virtuales:**
- `std::vector<PointType> read()` - Leer todos los puntos del archivo
- `std::vector<PointType> read_range(offset, count)` - Leer rango
- `size_t point_count()` - Número total de puntos
- `Box bbox()` - Bounding box del archivo
- `bool is_valid()` - Validar archivo

**Template Parameters:**
```cpp
template<typename Point_t = Point3f>
class FileReader { /*...*/ };

// Uso:
FileReader<Point3f> reader;        // Precisión simple
FileReader<Point3d> reader_double; // Precisión doble
```

---

### 📥 `las_file_reader.hpp`
Lector **secuencial** de archivos LAS.

**Características:**
- Lee puntos del archivo LAS en orden
- Soporta los estándares LAS 1.0 a 1.4
- Convierte datos binarios a structs Point
- Manejo de clasificaciones y metadatos

**Proceso:**
```
Archivo LAS (binario)
    ↓
Parseo de header (número de puntos, escala, offset)
    ↓
Lectura de registros (punto por punto)
    ↓
Conversión a (x, y, z) flotante
    ↓
Vector<Point>
```

**Métodos principales:**
- `read()` - Leer todo el archivo (vector en memoria)
- `reset()` - Volver al inicio
- `next()` - Siguiente punto (iterador)
- `filename()` - Nombre del archivo abierto
- `point_count()` - Puntos en archivo

**Validaciones:**
- Verificar firma LAS
- Validar número de puntos
- Chequear formato de registros
- Detectar errores de lectura

---

### 📥 `las_file_reader_parallel.hpp`
Lector **paralelo** de archivos LAS (con OpenMP).

**Características principales:**
- **Paralelización**: Divide el archivo en bloques, lectura paralela
- **Speedup**: Típicamente 4-8x más rápido (según cores)
- **Interfaz idéntica** a `las_file_reader.hpp`

**Estrategia:**
1. Determinar tamaño de punto del formato LAS
2. Dividir archivo en N bloques (1 por thread)
3. Leer bloques en paralelo
4. Consolidar resultados en vector final

```cpp
// Lectura paralela con 8 threads
#pragma omp parallel for num_threads(8)
for (int block = 0; block < num_blocks; ++block) {
    size_t offset = block * block_size;
    read_block(offset, block_size);  // En thread paralelo
}
```

**Ventajas:**
- ✅ Lectura mucho más rápida
- ✅ Aprovecha multiple cores
- ✅ Transparente para usuario
- ✅ Sin cambios de API

**Desventajas:**
- Requiere seek en archivo (no para stdin)
- Overhead para archivos muy pequeños

**Configuración:**
```cpp
LASFileReaderParallel reader(filename);
reader.set_num_threads(8);  // Especificar threads
auto points = reader.read();
```

---

### 🏭 `file_reader_factory.hpp`
Factory para crear lectores automáticamente.

**Responsabilidades:**
- Detectar tipo de archivo
- Crear reader apropiado
- Seleccionar entre secuencial/paralelo
- Cache de readers

**Uso:**
```cpp
auto reader = FileReaderFactory::create(filename);
auto points = reader->read();

// O con preferencia explícita:
auto reader = FileReaderFactory::create(
    filename, 
    FileReaderType::LAS_PARALLEL,
    8  // número de threads
);
```

**Heurísticas:**
- Archivos > 1GB → paralelo automáticamente
- Archivos < 100MB → secuencial
- Tamaño de punto → formato LAS

---

### 🔧 `handlers.hpp`
Manejadores para procesar puntos mientras se leen.

**Concepto:**
Aplicar transformaciones/filtros durante lectura:

```cpp
class PointHandler {
    virtual void handle(Point& p) = 0;  // Procesar punto
};

class TransformHandler : public PointHandler {
    void handle(Point& p) override {
        // Aplicar transformación de coordenadas
        p.x *= scale;
        p.y *= scale;
        p.z *= scale;
    }
};

class FilterHandler : public PointHandler {
    void handle(Point& p) override {
        // Filtrar puntos por criterio
        if (p.z < min_height) skip();
    }
};
```

**Ventajas:**
- Procesamiento on-the-fly (sin copia intermedia)
- Reducir memoria si se filtran puntos
- Normalización durante lectura

---

## Formatos LAS Soportados

| Aspecto | Detalles |
|---------|----------|
| **Versiones** | LAS 1.0, 1.1, 1.2, 1.3, 1.4 |
| **Escalas** | Automática detección desde header |
| **Offsets** | Soportados (transformación automática) |
| **Clasificaciones** | Leídas pero no usadas (opcional) |
| **RGB** | Opcional (ignorado en lectura básica) |
| **Intensidad** | Leída en metadatos (opcional) |

---

## Proceso Típico de Lectura

```cpp
// 1. Crear reader apropiado
auto reader = std::make_unique<LASFileReaderParallel>("data.las");

// 2. Validar archivo
if (!reader->is_valid()) {
    std::cerr << "Archivo inválido\n";
    return;
}

// 3. Obtener info
std::cout << "Puntos: " << reader->point_count() << "\n";
auto bbox = reader->bbox();

// 4. Leer datos
auto points = reader->read();

// 5. Usar puntos
auto octree = LinearOctree(points);
auto results = octree.knn_search(query, k);
```

---

## Optimizaciones

### 1. **Buffering**
- Buffer interno de lectura (típicamente 64KB)
- Reducir llamadas a syscalls
- Amortizar overhead de I/O

### 2. **Vectorización**
- Procesar múltiples puntos simultáneamente
- SIMD para conversiones de tipos
- SSE4/AVX cuando sea disponible

### 3. **Paralelización**
- Ver `las_file_reader_parallel.hpp`
- Típicamente 4-8x speedup

### 4. **Caché de Memory**
- Preallocate vector para evitar reallocations
- Estimar tamaño desde header
- Reservar con `reserve()` antes de `read_many()`

---

## Manejo de Errores

- **Archivo no encontrado**: Excepción `std::runtime_error`
- **Formato invalidó**: Detección de firma LAS
- **Corrupción de datos**: Validación de checksums (si aplica)
- **Lectura parcial**: Manejo graceful, retorna puntos leídos

---

## Comparación secuencial vs. Paralelo

| Operación | Secuencial | Paralelo | Speedup |
|-----------|-----------|----------|---------|
| 10K puntos | 10ms | 50ms * | 0.2x |
| 1M puntos | 500ms | 80ms | 6.25x |
| 10M puntos | 5s | 0.8s | 6.25x |
| 100M puntos | 50s | 8s | 6.25x |

*Overhead de paralelización para archivos pequeños

---

## Testing

Ver pruebas en:
- Validación de lectura correcta
- Preservación de precisión
- Acceleración paralela
- Manejo de archivos corruptos

---

## Notas de Implementación

- Manejo de endianness (arquitectura)
- Soporte para sparse point clouds (algunos puntos skipped)
- Memory-mapped I/O para archivos enormes (futuro)
- Streaming (lectura bajo demanda) en desarrollo

