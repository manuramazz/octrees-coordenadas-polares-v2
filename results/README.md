# `/results/` - Resultados de Benchmarking

Directorio que contiene todos los resultados de ejecuciones pasadas de benchmarks, organizados por tipo de benchmark y dataset.

## Estructura

```
results/
├── knn_full/                    # Búsquedas k-NN en dataset completo
├── knn_subset/                  # Búsquedas k-NN en subconjunto
├── full/                        # Búsquedas por radio en dataset completo
├── subset/                      # Búsquedas por radio en subconjunto
├── parallel_full/               # Paralelización en dataset completo
├── parallel_full_no_interleave/ # Paralelización sin interleaving
├── parallel_subset/             # Paralelización en subconjunto
├── parallel_subset_no_interleave/ # Paralelización sin interleaving (subset)
├── build_times/                 # Tiempos de construcción de estructuras
└── other/                       # Resultados variados/experimentales
```

---

## 📊 Directorios de Resultados

### `🔍 knn_full/`
Benchmarks de búsqueda **k-NN en dataset completo** (todos los puntos).

**Contenido típico:**
```
knn_full/
├── LinearOctree_knn_k10.json
├── LinearOctree_knn_k50.json
├── LinearOctree_knn_k100.json
├── OctreeReordered_cylindrical_knn_k10.json
├── nanoflann_knn_k10.json
├── PCL_Octree_knn_k10.json
└── ... (para cada estructura y parámetro)
```

**Métrica principal:** Tiempo de búsqueda (ms)

**Parámetros variados:**
- k = 10, 50, 100, 500, 1000, 5000
- Estructura = LinearOctree, OctreeReordered, Octree, nanoflann, PCL, PicoTree
- Reorder= None, Cylindrical, Spherical

---

### `🔍 knn_subset/`
Búsquedas **k-NN en subconjunto** de puntos (para pruebas rápidas).

**Similar a knn_full/ pero:**
- Menos puntos total → búsquedas más rápidas
- Útil para iteración rápida en desarrollo
- Tendencias similares a full pero escaladas

---

### `⭕ full/`
Benchmarks de **búsqueda por radio en dataset completo**.

**Contenido:**
```
full/
├── LinearOctree_radius_2.5.json
├── LinearOctree_radius_5.0.json
├── LinearOctree_radius_10.0.json
├── OctreeReordered_cylindrical_radius_5.0.json
└── ... (cada estructura y radio)
```

**Parámetros variados:**
- Radii = 2.5, 5.0, 7.5, 10.0, 15.0, 20.0 (unidades del dataset)
- Estructuras variadas

---

### `⭕ subset/`
Búsqueda por radio en **subconjunto** (análogo a knn_subset).

---

### `⚡ parallel_full/` y `parallel_subset/`
Benchmarks de **paralelización con OpenMP**.

**Contenido:**
```
parallel_full/
├── LinearOctree_threads_1.json
├── LinearOctree_threads_2.json
├── LinearOctree_threads_4.json
├── LinearOctree_threads_8.json
└── ... (variando número de threads)
```

**Métrica:** Impacto de paralelización en speedup

**Análisis típico:**
```python
# Speedup = tiempo_serial / tiempo_paralelo
speedup_8threads = time_1thread / time_8threads
# Ideal: ~8x para 8 threads (eficiencia de paralelización)
```

---

### `🏗️ build_times/`
Benchmarks de **construcción de estructuras**.

**Contenido:**
```
build_times/
├── LinearOctree_build.json
├── OctreeReordered_cylindrical_build.json
├── Octree_build.json
├── PCL_Octree_build.json
└── ...
```

**Métricas:**
- Tiempo de codificación (Morton/Hilbert)
- Tiempo de ordenamiento
- Tiempo de construcción de estructura
- Tiempo total

---

### `🔧 other/`
Resultados experimentales o variados.

Puede contener:
- Pruebas de nuevas características
- Experimentos fallidos (para referencia)
- Datos de debugging
- Comparativas puntuales

---

## Formato JSON de Resultados

Ejemplo de archivo de resultado:

```json
{
  "metadata": {
    "date": "2024-03-15T14:32:00Z",
    "dataset": "Paris_Luxembourg_6.las",
    "dataset_size_points": 1000000,
    "structure": "LinearOctree",
    "encoder": "MORTON_3D",
    "reorder_mode": "None",
    "num_queries": 100
  },
  
  "build": {
    "encoding_time_ms": 234.5,
    "sorting_time_ms": 12.3,
    "structure_build_time_ms": 45.6,
    "total_build_time_ms": 292.4
  },
  
  "search_radius_5.0": {
    "radius": 5.0,
    "query_count": 100,
    "individual_times_ms": [0.45, 0.48, 0.42, ...],
    "total_time_ms": 45.23,
    "avg_time_ms": 0.4523,
    "median_time_ms": 0.45,
    "std_dev_ms": 0.02,
    "min_time_ms": 0.40,
    "max_time_ms": 0.58,
    "results_per_query": [234, 245, 230, ...]
  },
  
  "knn_k_10": {
    "k": 10,
    "query_count": 100,
    "individual_times_ms": [...],
    "total_time_ms": 52.31,
    "avg_time_ms": 0.5231,
    ...
  }
}
```

---

## Scripts para Generar Resultados

Desde la raíz del proyecto:

### Buscar por radio
```bash
./bench_neighbors.bash
# Genera: results/full/, results/subset/
```

### Analizar localidad de caché
```bash
./bench_locality.bash
# Genera: resultados de PAPI, análisis de cache
```

### Profiling de memoria
```bash
./bench_memory.bash
# Genera: heaptrack/, tamaños de estructura
```

---

## Análisis con Python

### Cargar y analizar resultados

```python
import json
import pandas as pd

# Cargar un resultado
with open('results/full/LinearOctree_radius_5.0.json') as f:
    data = json.load(f)

# Extraer información
times = data['search_radius_5.0']['individual_times_ms']
avg = data['search_radius_5.0']['avg_time_ms']

print(f"Tiempo promedio: {avg:.3f} ms")
print(f"Min: {min(times):.3f} ms, Max: {max(times):.3f} ms")
print(f"Desviación: {pd.Series(times).std():.3f} ms")
```

### Comparar estructuras

```python
import json
import matplotlib.pyplot as plt

structures = ['LinearOctree', 'OctreeReordered', 'PCL_Octree', 'nanoflann']
radii = [2.5, 5.0, 10.0]
results = {}

for struct in structures:
    times = []
    for r in radii:
        fname = f'results/full/{struct}_radius_{r}.json'
        with open(fname) as f:
            data = json.load(f)
            times.append(data[f'search_radius_{r}']['avg_time_ms'])
    results[struct] = times

# Graficar
for struct, times in results.items():
    plt.plot(radii, times, label=struct)
plt.xlabel('Radio (unidades)')
plt.ylabel('Tiempo (ms)')
plt.legend()
plt.show()
```

---

## Limpieza de Antiguos Resultados

```bash
# Respaldar resultados antiguos
tar czf results_backup_$(date +%Y%m%d).tar.gz results/

# Limpiar
rm -rf results/*

# Ejecutar benchmarks nuevamente
./bench_neighbors.bash
```

---

## Interpretación de Resultados

### Velocidad

More rápido = tiempo_ms más bajo:
```
LinearOctree: 0.45 ms          ← Rápido ✓
OctreeReordered: 0.38 ms       ← Más rápido ✓✓
PCL_Octree: 0.92 ms            ← Lento
```

### Escalabilidad

Examinar cómo varía con parámetros:
```python
# Escalabilidad con k
k_values = [10, 50, 100, 500]
times = [0.45, 0.67, 1.05, 2.1]  # Tendencia: sublineal es bueno

# Esperado: O(log N + k)  ← búsqueda logarítmica + k salidas
```

###Localidad de Caché

Resultados PAPI deberían mostrar:
- **Sin reordenamiento**: cache hit rate ~ 60-70%
- **Con Morton**: cache hit rate ~ 75-85%
- **Con Hilbert**: cache hit rate ~ 80-90%

---

## Reproducibilidad

Para resultados reproducibles:
1. Usar mismo dataset
2. Usar mismo compilador/flags
3. Usar mismo hardware (o normalizar)
4. Usar mismo número de queries
5. Promediar múltiples ejecuciones

---

## Notas

- Resultados pueden variar con **sistema/hardware**
- Tests múltiples (varianza normal de ~5-10%)
- Perfiles con **gráficos** en `plots/`
- Mantener histórico para seguimiento de optimizaciones

