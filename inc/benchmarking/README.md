# `/inc/benchmarking/` - Sistema de Medición de Rendimiento

Módulo que proporciona la infraestructura completa para medir y registrar el rendimiento de las distintas estructuras de datos y algoritmos de búsqueda espacial.

## Archivos Principales

### 📋 `benchmarking.hpp`
Clase base abstracta `Benchmark<ResultType>` que define la interfaz general para ejecutar benchmarks.

**Responsabilidades:**
- Define estructura para ejecutar y registrar benchmarks
- Proporciona métodos para medir tiempos de ejecución
- Registra resultados en archivos JSON o CSV

**Uso típico:**
```cpp
class MiBenchmark : public Benchmark<MiResultado> {
    ResultType run() override { /*...*/ }
};
```

---

### 📊 `neighbor_benchmarks.hpp`
Benchmarks para búsquedas de vecinos (kNN y búsqueda por radio).

**Contenido:**
- `NeighborBenchmark` - Clase base para benchmarks de vecinos
- Medición de tiempos de búsqueda
- Validación de resultados correctos
- Registro de estadísticas

**Métricas medidas:**
- Tiempo total de búsqueda
- Tiempo promedio por punto consultado
- Número de puntos devueltos
- Consistencia de resultados

---

### 🏗️ `enc_build_benchmarks.hpp`
Benchmarks para la construcción de estructuras y codificación de puntos.

**Funcionalidad:**
- Medición de tiempo de codificación (Morton/Hilbert)
- Medición de tiempo de construcción de estructuras
- Análisis del overhead de construcción
- Comparación entre diferentes codificadores

**Métricas principales:**
- Tiempo de codificación de N puntos
- Tiempo de construcción de estructura
- Throughput de codificación (puntos/seg)

---

### 💾 `memory_benchmarks.hpp`
Medición del uso de memoria de diferentes estructuras.

**Características:**
- Medición directa de uso de memoria
- Comparación de tamaño entre structures (Octree lineal vs. con punteros)
- Análisis de fragmentación
- Ratios de compresión

**Información registrada:**
- Bytes usados por la estructura
- Bytes por punto
- Comparación relativa vs. estructura de referencia

---

### 📍 `memory_benchmarks.hpp` (continuación)
Profiling de localidad de caché en búsquedas.

**Contenido:**
- Análisis de accesos a caché L1, L2, L3
- Comparación de diferentes órdenes de puntos
- Efecto de Morton vs. Hilbert en localidad
- Hit/miss rates de caché

**Métricas medidas:**
- Cache hits/misses
- Cache line utilization
- Principales diferencias entre curvas SFC

---

### ⏱️ `time_watcher.hpp`
Cronómetro de precisión para medir tiempos de ejecución.

**Características:**
- Medición de tiempo en nanosegundos
- Soporte para múltiples puntos de medición
- Métodos para iniciar/detener temporizadores
- Estadísticas de tiempos

**Uso típico:**
```cpp
TimeWatcher watcher;
watcher.start();
// código a medir
watcher.stop();
double ms = watcher.elapsed_milliseconds();
```

---

### 📝 `encoding_log.hpp`
Registro de información sobre la codificación de puntos.

**Registra:**
- Tipo de codificador utilizado
- Tiempo de codificación
- Rango de valores
- Número de puntos codificados
- Estadísticas de distribución

---

### 📝 `build_log.hpp`
Registro de información sobre la construcción de estructuras.

**Registra:**
- Tipo de estructura
- Tiempo de construcción
- Profundidad del árbol
- Número de nodos
- Número de hojas
- Estadísticas de distribución de tamaños

---

### 📊 `search_set.hpp`
Define conjuntos de puntos para realizar búsquedas durante benchmarks.

**Contenido:**
- `SearchPoint` - Estructura para punto de búsqueda (origen)
- `SearchSet` - Conjunto de puntos de búsqueda
- Generadores de conjuntos aleatorios
- Generadores de puntos de búsqueda sistemáticos

**Propósito:**
- Permitir benchmarks reproducibles
- Variar parámetros de búsqueda (radios, k valores)
- Evaluar rendimiento en diferentes escenarios

---

### 🔍 `papi_events.hpp`
Integración con PAPI (Performance API) para profiling detallado.

**Funcionalidad:**
- Medición de eventos de CPU (cache misses, flops, etc.)
- Denominador de caché L1, L2, L3
- Branch predictions
- Instrucciones por ciclo

**Eventos monitoreados:**
- L1_DCM (cache misses L1)
- L2_DCM (cache misses L2)
- LL_DCM (cache misses L3)
- PAPI_BR_MSP (branch mispredictions)

---

## Flujo Típico de Benchmarking

1. **Preparación**: Cargar datos con `LASFileReader`
2. **Codificación**: Aplicar `PointEncoder` (Morton/Hilbert)
3. **Construcción**: Crear estructura (`LinearOctree`, `Octree`, etc.)
4. **Medición**: Ejecutar `NeighborBenchmark` con `TimeWatcher`
5. **Profiling**: Recolectar eventos con `PAPI` (opcional)
6. **Registro**: Guardar resultados con `BuildLog`/`EncodingLog`
7. **Análisis**: Procesar resultados en Python

---

## Uso de Benchmarks en el Proyecto

Los benchmarks se utilizan principalmente a través de:
- Script `bench_neighbors.bash` - Búsquedas de vecinos
- Script `bench_memory.bash` - Profiling de memoria
- Script `bench_locality.bash` - Análisis de localidad de caché

---

## Notas Técnicas

- Todos los tiempos se miden con `std::chrono` de precisión nanosegundos
- PAPI es opcional; código falla gracefully sin él
- OpenMP se usa para paralelizar mediciones cuando sea posible
- Resultados se exportan en formato JSON para procesamiento en Python

