# `/cmake/modules/` - Módulos de Búsqueda Personalizados

Directorio que contiene módulos CMake personalizados (`FindXXX.cmake`) para buscar e integrar librerías externas opacionales.

## archivos

```
modules/
├── FindLASLIB.cmake      # Búsqueda de LASlib
├── FindPCL.cmake        # Búsqueda de Point Cloud Library
├── FindPapi.cmake       # Búsqueda de PAPI
└── FindPicotree.cmake   # Búsqueda de PicoTree
```

---

## 📦 `FindLASLIB.cmake`
Módulo para encontrar **LASlib** (librería para leer archivos LAS).

**Responsabilidades:**
- Buscar headers de LASlib (`lasreader.hpp`)
- Buscar librería compilada (`liblas.a` o `liblas.so`)
- Definir variables `LASLIB_FOUND`, `LASLIB_INCLUDE_DIRS`, `LASLIB_LIBRARIES`
- Crear target CMake para linking

**Ubicaciones búsquedas:**
- Rutas estándar del sistema
- `${CMAKE_PREFIX_PATH}`
- Directorios especificados en `LASLIB_ROOT` (si se pasa)

**Uso en CMakeLists.txt:**
```cmake
find_package(LASLIB REQUIRED)
target_link_libraries(mi_proyecto ${LASLIB_LIBRARIES})
target_include_directories(mi_proyecto PRIVATE ${LASLIB_INCLUDE_DIRS})
```

**Instalación de LASlib:**
Ver [scripts/install_laslib.sh](../scripts/README.md)

---

## 📦 `FindPCL.cmake`
Módulo para encontrar **Point Cloud Library** (opcional, para comparación).

**Características:**
- Buscar versión específica (1.13+)
- Encontrar headers y librerías de PCL
- Integración con componentes (octree, kdtree, etc.)

**Ubicación típica:**
- `$HOME/local/pcl/` (según install_pcl.sh)
- Rutas estándar del sistema

**Uso:**
```cmake
find_package(PCL 1.13 QUIET)
if(PCL_FOUND)
    add_definitions(-DWITH_PCL)
    target_link_libraries(proyecto ${PCL_LIBRARIES})
endif()
```

**Si no se encuentra:**
- Compilación continúa sin PCL
- Mensajes de warning indicando ausencia
- Benchmarks de PCL simplemente no se ejecutan

---

## 📦 `FindPapi.cmake`
Módulo para encontrar **PAPI** (Performance API, opcional).

**Características:**
- Buscar libería PAPI (`libpapi.a` o `libpapi.so`)
- Encontrar headers PAPI
- Verificar que PAPI está correctamente instalado

**Ubicación típica:**
- `lib/papi/` (según install_papi.sh)
- Rutas estándar del sistema

**Requisitos del Sistema:**
- Linux con kernel moderno (soporte contadores de CPU)
- Acceso a `/sys/devices/pmu/` (típicamente requiere permisos)
- Kernel tuning: `echo 2 > /proc/sys/kernel/perf_event_paranoid`

**Uso:**
```cmake
if(ENABLE_PAPI)
    find_package(PAPI REQUIRED)
    add_definitions(-DWITH_PAPI)
    target_link_libraries(proyecto ${PAPI_LIBRARIES})
endif()
```

---

## 📦 `FindPicotree.cmake`
Módulo para encontrar **PicoTree** (structure de comparación, opcional).

**Características:**
- Buscar headers de PicoTree
- Librería header-only (no requiere compilación)

**Ubicación:**
- `lib/picotree/` (según install_picotree.sh)
- Include paths estándar

**Uso:**
```cmake
find_package(PicoTree QUIET)
if(PicoTree_FOUND)
    add_definitions(-DWITH_PICOTREE)
endif()
```

---

## Cómo Funcionan los Módulos FindXXX

### Estructura típica

```cmake
# FindMILIBRERA.cmake

# Búsqueda
find_path(MILIBRERIA_INCLUDE_DIR NAMES header.h)
find_library(MILIBRERIA_LIBRARY NAMES mylib)

# Determinar si se encontró
if(MILIBRERIA_INCLUDE_DIR AND MILIBRERIA_LIBRARY)
    set(MILIBRERIA_FOUND TRUE)
else()
    set(MILIBRERIA_FOUND FALSE)
endif()

# Exportar variables
set(MILIBRERIA_INCLUDE_DIRS ${MILIBRERIA_INCLUDE_DIR})
set(MILIBRERIA_LIBRARIES ${MILIBRERIA_LIBRARY})

# Crear target (mejor práctica)
if(MILIBRERIA_FOUND AND NOT TARGET MILIBRERIA::MILIBRERIA)
    add_library(MILIBRERIA::MILIBRERIA INTERFACE)
    target_include_directories(MILIBRERIA::MILIBRERIA INTERFACE ${MILIBRERIA_INCLUDE_DIRS})
    target_link_libraries(MILIBRERIA::MILIBRERIA INTERFACE ${MILIBRERIA_LIBRARIES})
endif()
```

### Variables CMake Generadas

Para cada librería XXX:
- `XXX_FOUND` - TRUE si se encontró
- `XXX_INCLUDE_DIRS` - Directorios de headers
- `XXX_LIBRARIES` - Archivos .a o .so a linkear
- `XXX_VERSION` - Versión (si está disponible)

---

## Uso en CMakeLibraries.cmake

```cmake
# Requeridas
find_package(LASLIB REQUIRED)

# Opcionales
find_package(PCL QUIET)
if(NOT PCL_FOUND)
    message(WARNING "PCL no encontrado, benchmarks de PCL deshabilitados")
endif()

# Condicionales
if(ENABLE_PAPI)
    find_package(PAPI REQUIRED)
else()
    message(STATUS "PAPI deshabilitado, profiling de caché no disponible")
endif()

# Linking
target_link_libraries(octrees-benchmark
    PRIVATE
        ${LASLIB_LIBRARIES}
        ${PCL_LIBRARIES}
        ${PAPI_LIBRARIES}
)
```

---

##Rutas Customizadas

Si CMake no encuentra una librería, especificar ruta manualmente:

```bash
cmake -B build \
    -DLASLIB_ROOT=/custom/path/to/laslib \
    -DPCL_DIR=/custom/path/to/pcl \
    .
```

O establecer variable de entorno:
```bash
export CMAKE_PREFIX_PATH=/custom/path:$CMAKE_PREFIX_PATH
cmake -B build .
```

---

## Troubleshooting

### "Could not find XXX"

1. Verificar que la librería está instalada:
   ```bash
   ls /usr/local/include/  # Buscar headers
   ls /usr/local/lib/      # Buscar librerías
   ```

2. Instalar si falta:
   ```bash
   ./scripts/install_xxxx.sh
   ```

3. Especificar ruta manualmente (ver arriba)

### Errores de linking

Si se encuentran headers pero no linking:
- Verificar que librería compilada existe
- Verificar permisos de lectura
- Usar `ldd` para checker dependencias

### Versión incorrecta

CMake buscapor defecto versión estándar. Para especificar:
```cmake
find_package(PCL 1.15 EXACT REQUIRED)
```

---

## Notas

- Módulos son **reutilizables** en otros proyectos
- Filosofía: **fail gracefully** para opcionales
- Mejor que hardcodear rutas
- Facilita desarrollo en múltiples máquinas/entornos

