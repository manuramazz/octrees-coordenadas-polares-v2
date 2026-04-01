# Índice Completo de Documentación

Este archivo proporciona un índice centralizado de toda la documentación del proyecto en español.

## 📚 Documentación Principal

- **[README.md](README.md)** - Descripción general del proyecto, quick start, opciones principales

## 📁 Documentación por Directorio

### Código Fuente

| Directorio | Descripción | Documentación |
|-----------|-------------|---------------|
| `inc/` | Headers y librerías (templates) | [inc/README.md](inc/README.md) |
| `inc/benchmarking/` | Sistema de medición de rendimiento | [README](inc/benchmarking/README.md) |
| `inc/encoding/` | Codificadores SFC (Morton, Hilbert 3D) | [README](inc/encoding/README.md) |
| `inc/structures/` | **Octrees linealizados y reordenados** | [README](inc/structures/README.md) |
| `inc/geometry/` | Primitivas geométricas (Point, Box) | [README](inc/geometry/README.md) |
| `inc/kernels/` | Kernels de búsqueda spa cial | [README](inc/kernels/README.md) |
| `inc/readers/` | Lectores de archivos LAS (secuencial/paralelo) | [README](inc/readers/README.md) |
| `src/` | Implementación en C++ (.cpp) | [src/README.md](src/README.md) |

### Testing y Build

| Directorio | Descripción | Documentación |
|-----------|-------------|---------------|
| `tests/` | Suite de pruebas (GoogleTest) | [tests/README.md](tests/README.md) |
| `tests/structures/` | Tests de octrees y estructuras | [README](tests/structures/README.md) |
| `tests/encoding/` | Tests de codificadores | [README](tests/encoding/README.md) |
| `tests/geometry/` | Tests de primitivas geom | [README](tests/geometry/README.md) |
| `tests/integration/` | Tests end-to-end | [README](tests/integration/README.md) |
| `cmake/` | Configuración de build (CMake) | [cmake/README.md](cmake/README.md) |
| `cmake/modules/` | Módulos personalizados FindXXX | [README](cmake/modules/README.md) |

### Utilidades y Datos

| Directorio | Descripción | Documentación |
|-----------|-------------|---------------|
| `scripts/` | Scripts de instalación de dependencias | [scripts/README.md](scripts/README.md) |
| `examples/` | Ejemplo de uso de la librería | [examples/README.md](examples/README.md) |
| `data/` | Datasets LAS (Paris, Lille) | [data/README.md](data/README.md) |
| `results/` | Resultados de benchmarks (JSON) | [results/README.md](results/README.md) |
| `plots/` | Análisis Python + Jupyter | [plots/README.md](plots/README.md) |

---

## 🚀 Guía de Usuario

### Para comenzar
1. Ver [README.md](README.md) - Descripción y quick start
2. Ver [scripts/README.md](scripts/README.md) - Instalar dependencias
3. Ver [examples/README.md](examples/README.md) - Ejemplo simple

### Para entender el código
1. Ver [inc/README.md](inc/README.md) - Visión general de librerías
2. Ver directorios específicos bajo `inc/`
3. Ver [tests/README.md](tests/README.md) - Tests como referencia

### Para desarrollar
1. Ver [cmake/README.md](cmake/README.md) - Sistema de build
2. Ver [src/README.md](src/README.md) - Implementación
3. Ver [tests/README.md](tests/README.md) - Testing
4. Ver directorios específicos bajo `tests/`

### Para analizar resultados
1. Ejecutar benchmarks: Ver [README.md](README.md) sección "Scripts de Benchmark"
2. Ver [results/README.md](results/README.md) - Formato de resultados
3. Ver [plots/README.md](plots/README.md) - Análisis y visualización

---

## 📊 Contenido por Tema

### Óctrees y Estructuras de Datos
- Descripción general: [inc/structures/README.md](inc/structures/README.md)
- Búsqueda: [inc/kernels/README.md](inc/kernels/README.md)
- Tests: [tests/structures/README.md](tests/structures/README.md)

### Codificación y Curvas
- Teoría: [inc/encoding/README.md](inc/encoding/README.md)
- Tests: [tests/encoding/README.md](tests/encoding/README.md)

### Geometría
- Primitivas: [inc/geometry/README.md](inc/geometry/README.md)
- Tests: [tests/geometry/README.md](tests/geometry/README.md)

### Benchmarking y Análisis
- Medición: [inc/benchmarking/README.md](inc/benchmarking/README.md)
- Resultados: [results/README.md](results/README.md)
- Visualización: [plots/README.md](plots/README.md)

### Entrada/Salida
- Lectura de datos: [inc/readers/README.md](inc/readers/README.md)
- Datasets: [data/README.md](data/README.md)

---

## 🔍 Búsqueda Rápida

**¿Cómo...?**

| Pregunta | Respuesta |
|----------|-----------|
| ...instalar el proyecto? | [README.md](README.md) sección Installation |
| ...ejecutar un benchmark? | [README.md](README.md) sección "Scripts de Benchmark" |
| ...entender el octree linealizado? | [inc/structures/README.md](inc/structures/README.md) |
| ...usar diferentes codificadores? | [inc/encoding/README.md](inc/encoding/README.md) |
| ...buscar vecinos en el código? | [inc/kernels/README.md](inc/kernels/README.md) |
| ...cargar archivos LAS? | [inc/readers/README.md](inc/readers/README.md) |
| ...medir el rendimiento? | [inc/benchmarking/README.md](inc/benchmarking/README.md) |
| ...ejecutar tests? | [tests/README.md](tests/README.md) |
| ...analizar resultados? | [plots/README.md](plots/README.md) |
| ...compilar el proyecto? | [cmake/README.md](cmake/README.md) |
| ...ver un ejemplo simple? | [examples/README.md](examples/README.md) |
| ...entender la geometría (Point, Box)? | [inc/geometry/README.md](inc/geometry/README.md) |

---

## 📝 Notas

- Toda la documentación está en **español**
- Cada README incluye explicaciones técnicas detalladas
- Ejemplos de código incluidos donde relevante
- Links cruzados entre documentos relacionados
- Últimas actualizaciones: Marzo 2024

---

## 🎯 Resumen Rápido del Proyecto

**octrees-benchmark**: Proyecto de investigación que implementa **octrees linealizados** con **reordenamiento en coordenadas polares** para búsquedas rápidas de vecinos en datos LiDAR.

**Componentes principales:**
1. ✅ Octrees linealizados (sin punteros)
2. ✅ Curvas que llenan el espacio (Morton y Hilbert 3D)
3. ✅ Reordenamiento en coordenadas cilíndricas/esféricas
4. ✅ Búsquedas de vecinos (kNN y radius search)
5. ✅ Benchmarking exhaustivo
6. ✅ Análisis de localidad de caché con PAPI
7. ✅ Visualización de resultados en Python

