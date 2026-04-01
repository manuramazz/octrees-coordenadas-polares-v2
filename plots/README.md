# `/plots/` - Visualización y Análisis de Resultados

Directorio que contiene notebooks de Jupyter y scripts Python para visualizar y analizar los resultados de benchmarks del proyecto.

## Estructura

```
plots/
├── plots_benchmarks.ipynb      # Análisis de tiempos de búsqueda
├── plots_locality.ipynb        # Análisis de localidad de caché
├── plots_encodings.ipynb       # Comparación de codificadores
├── plots_clouds.ipynb          # Visualización de nubes de puntos
├── plots.py                    # Funciones de gráficos reutilizables
├── utils.py                    # Utilidades para procesamiento de datos
└── constants.py                # Constantes de configuración
```

---

## Notebooks Jupyter

### 📊 `plots_benchmarks.ipynb`
Análisis exhaustivo de benchmarks de búsqueda de vecinos.

**Contenido:**
- Carga de resultados JSON desde `results/`
- Comparación de tiempos de búsqueda
- Gráficos de rendimiento:
  - Tiempo por radio vs. estructura
  - Tiempo por k-valor vs. estructura
  - Distribución de tiempos
  - Speedup relativo
- Estadísticas (media, desviación estándar, percentiles)
- Pruebas estadísticas (ANOVA, t-test)

**Métricas principales:**
- **Tiempo total** de N búsquedas
- **Tiempo promedio** por búsqueda
- **Throughput**: búsquedas por segundo
- **Varianza**: desviación en tiempos

**Gráficos típicos:**
```python
# Línea: Tiempo vs. Radio (para cada estructura)
for structure in structures:
    plot(radii, times[structure], label=structure)
    
# Barras: Comparación de estructuras para radio fijo
bar_chart(structures, times_at_radius_5)

# Heatmap: Tiempo vs. (Estructura, Radio)
heatmap(times_matrix)
```

---

### 🎯 `plots_locality.ipynb`
Análisis de localidad de caché y eficiencia de memoria.

**Contenido:**
- Datos de PAPI (cache hits/misses)
- Comparación Morton vs. Hilbert
- Comparación de modos de reordenamiento (None, Cylindrical, Spherical)
- Impacto en localidad de caché

**Métricas:**
- **Cache miss rate**: % de accesos fallidos
- **Cache hit rate**: % de accesos exitosos
- **L1/L2/L3 misses**: Por nivel de caché
- **Branch misprediction rate**

**Análisis:**
- Cómo el reordenamiento mejora localidad
- Trade-off: tiempo codificación vs. mejora cache
- Efectividad de diferentes SFCs

---

### 🔢 `plots_encodings.ipynb`
Comparación de diferentes codificadores de curvas que llenan el espacio.

**Contenido:**
- Tiempo de codificación (codificador vs. núm. puntos)
- Comparación de códigos generados
- Preservación de proximidad (verificación)
- Vectorización vs. serial

**Métricas:**
- **Throughput de codificación**: puntos/segundo
- **Precisión**: error de reconstrucción
- **Localidad**: proximidad preservada

---

### ☁️ `plots_clouds.ipynb`
Visualización 3D de nubes de puntos y análisis geométrico.

**Contenido:**
- Visualización 3D de datos LAS
- Distribución espacial de puntos
- Identificación de clusters
- Escala de colores por densidad/altura

**Herramientas:**
- Matplotlib 3D
- Plotly (interactivo)
- Mayavi (si disponible)

---

## Scripts Python

### 📈 `plots.py`
Funciones reutilizables para generar gráficos.

**Funciones principales:**

```python
# Gráficos básicos
def plot_benchmark_comparison(data, title, xlabel, ylabel):
    """Gráfico de línea comparando estructuras."""
    
def plot_bar_chart(structures, values, title):
    """Gráfico de barras para comparación directa."""
    
def plot_heatmap(data, title, xlabel, ylabel):
    """Heatmap para relaciones multidimensionales."""
    
# Gráficos especializados
def plot_cache_metrics(papi_data, structures):
    """Gráficos de métricas de caché."""
    
def plot_locality_improvement(reorder_modes):
    """Mejora de localidad por modo de reordenamiento."""
    
def plot_knn_performance(k_values, times):
    """Rendimiento variando k."""
```

**Styling:**
- Colores consistentes
- Leyendas claras
- Etiquetas de ejes informativas
- Resolución alta (DPI)

---

### 🛠️ `utils.py`
Funciones de utilidad para procesamiento de datos.

**Contenido:**

```python
# Carga de datos
def load_results_json(filepath):
    """Cargar resultados desde archivo JSON."""
    return json.load(open(filepath))

def load_all_results(directory):
    """Cargar todos los resultados de un directorio."""
    
# Procesamiento
def aggregate_results(results_list):
    """Agregar resultados de múltiples ejecuciones."""
    
def normalize_times(times):
    """Normalizar tiempos para comparación relativa."""
    
def calculate_statistics(data):
    """Estadísticas: media, std, percentiles."""
    return {
        'mean': np.mean(data),
        'std': np.std(data),
        'min': np.min(data),
        'max': np.max(data),
        'p50': np.percentile(data, 50),
        'p95': np.percentile(data, 95)
    }

# Validación
def validate_results(results):
    """Verificar integridad de resultados."""
    
def compare_structures(results1, results2):
    """Comparar resultados de dos estructuras."""
```

---

### ⚙️ `constants.py`
Configuración global para análisis.

**Contenido:**

```python
# Paths
RESULTS_DIR = '../results/'
DATA_DIR = '../data/'
OUTPUT_DIR = './output/'

# Nombres de estructuras
STRUCTURES = [
    'LinearOctree',
    'OctreeReordered',
    'PtrOctree',
    'nanoflann_KD',
    'PCL_Octree',
    'PCL_KD',
    'PicoTree'
]

# Colores para gráficos
COLORS = {
    'LinearOctree': '#1f77b4',
    'OctreeReordered': '#ff7f0e',
    # ...
}

# Parámetros de gráficos
FIGURE_SIZE = (12, 6)
DPI = 300
FONT_SIZE = 12

# Configuración de análisis
INCLUDE_WARMUP = False
PERCENTILE_THRESHOLD = 95
MIN_RUNS = 5
```

---

## Uso Típico

### Ejecutar Notebook en Jupyter

```bash
# Instalar dependencias (una sola vez)
pip install jupyter numpy pandas matplotlib plotly scipy

# Iniciar servidor Jupyter
cd plots
jupyter notebook

# En el navegador: click en plots_benchmarks.ipynb
```

### Generar gráficos desde línea de comandos

```python
# script_gen_plots.py
from plots import *
from utils import *
from constants import *

# Cargar datos
results = load_all_results(RESULTS_DIR)

# Generar gráficos
plot_benchmark_comparison(results, 'Benchmark Comparación')
plt.savefig(OUTPUT_DIR + 'benchmark.png', dpi=DPI)

plot_cache_metrics(results, STRUCTURES)
plt.savefig(OUTPUT_DIR + 'cache.png', dpi=DPI)

plt.show()
```

---

## Estructura de Datos JSON de Resultados

Ejemplo de resultado guardado en `results/benchmark.json`:

```json
{
  "metadata": {
    "date": "2024-03-15",
    "dataset": "Paris_Luxembourg_6.las",
    "num_points": 1000000
  },
  "structures": [
    {
      "name": "LinearOctree",
      "build_time_ms": 45.3,
      "searches": [
        {
          "radius": 2.5,
          "query_count": 100,
          "times_ms": [0.5, 0.52, 0.48, ...],
          "total_ms": 50.2,
          "avg_ms": 0.502,
          "median_ms": 0.50
        },
        {
          "radius": 5.0,
          "query_count": 100,
          "times_ms": [...],
          "total_ms": 102.1,
          "avg_ms": 1.021
        }
      ]
    }
  ]
}
```

---

## Workflow Típico de Análisis

1. **Ejecutar benchmarks** (desde raíz del proyecto):
   ```bash
   ./bench_neighbors.bash  # Genera resultados/*.json
   ```

2. **Abrir notebook**:
   ```bash
   cd plots
   jupyter notebook plots_benchmarks.ipynb
   ```

3. **Modificar análisis según resultados**:
   - Cambiar rangos de radios
   - Agregar nuevas estructuras
   - Realizar análisis estadísticos adicionales

4. **Exportar gráficos**:
   ```python
   # En notebook
   plt.savefig('output/mi_grafico.png', dpi=300, bbox_inches='tight')
   ```

---

## Dependencias Python

```bash
pip install \
    jupyter \           # Notebooks interactivos
    numpy \             # Computación numérica
    pandas \            # Manipulación de datos
    matplotlib \        # Gráficos estáticos
    plotly \            # Gráficos interactivos
    scipy \             # Estadísticas
    seaborn             # Gráficos estadísticos mejorados
```

---

## Notas

- Los scripts son **Python 3.8+**
- Notebooks automáticamente generan gráficos inline
- Resultados se cachean para no recargar cada vez
- Análisis reproducible (seeds fijas para RNG cuando sea relevante)

