# `/cmake/` - Configuración de Build del Proyecto

Directorio que contiene toda la configuración de CMake para compilar el proyecto de forma modular y flexible.

## Estructura

```
cmake/
├── CMakeLists.txt          # Orquestador principal
├── CMakeSources.cmake      # Recopilación de archivos fuente
├── CMakeLibraries.cmake    # Búsqueda y vinculación de librerías
├── CMakeBuildExec.cmake    # Configuración del ejecutable
├── CMakeConfig.cmake       # Opciones de compilación
└── modules/                # Módulos personalizados (FindXXX)
    ├── FindLASLIB.cmake
    ├── FindPCL.cmake
    ├── FindPapi.cmake
    └── FindPicotree.cmake
```

---

## Archivos Principales

### 🔧 `CMakeLists.txt`
**Archivo principal de CMake** - Punto de entrada para `cmake` command.

**Responsabilidades:**
1. Definir versión de proyecto y requerimientos
2. Establecer opciones de build globales
3. Cargar configuraciones modulares
4. Definir targets (ejecutable, librería, tests)

**Contenido típico:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(octrees-benchmark VERSION 1.0 LANGUAGES CXX)

# Opciones del usuario
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_PAPI "Enable PAPI profiling" OFF)
option(DEBUG "Enable debug symbols" OFF)

# Cargar configuraciones modulares
include(cmake/CMakeSources.cmake)
include(cmake/CMakeLibraries.cmake)
include(cmake/CMakeBuildExec.cmake)

# Si tests habilitados
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

**Variables importantes:**
- `CMAKE_CXX_STANDARD` - C++ version (17, 20, etc)
- `CMAKE_CXX_FLAGS_RELEASE` - Flags para Release
- `CMAKE_BUILD_TYPE` - Debug, Release, RelWithDebInfo
- `CMAKE_EXPORT_COMPILE_COMMANDS` - Generar compile_commands.json

---

### 📝 `CMakeSources.cmake`
**Recopilación de archivos fuente** del proyecto.

**Contenido:**
```cmake
# Headers (template-only)
set(HEADERS
    inc/main_options.hpp
    inc/util.hpp
    inc/benchmarking/*.hpp
    inc/encoding/*.hpp
    inc/structures/*.hpp
    inc/geometry/*.hpp
    inc/kernels/*.hpp
    inc/readers/*.hpp
)

# Implementación (.cpp)
set(SOURCES
    src/main.cpp
    src/main_options.cpp
    src/octree.cpp
    src/time_watcher.cpp
    src/papi_events.cpp
)

# Excluir main.cpp para librería
set(LIBRARY_SOURCES ${SOURCES})
list(REMOVE_ITEM LIBRARY_SOURCES src/main.cpp)
```

**Características:**
- Wildcard patterns para headers
- Separación de sources ejecutable vs librería
- Fácil agregar nuevos archivos (.src/*.cpp se incluye automáticamente)

---

### 📚 `CMakeLibraries.cmake`
**Búsqueda y vinculación de dependencias externas**.

**Contenido:**
```cmake
# Obligatorias
find_package(OpenMP REQUIRED)

# Opcionales con fallback
find_package(Eigen3 REQUIRED)
find_package(LASlib REQUIRED)      # Custom module (modules/FindLASLIB.cmake)

if(NOT LASlib_FOUND)
    message(WARNING "LASlib no encontrado, algunas funciones deshabilitadas")
endif()

# Opcional - PCL
find_package(PCL 1.13 QUIET)
if(PCL_FOUND)
    list(APPEND COMPILE_DEFINITIONS WITH_PCL)
endif()

# Opcional - PAPI
if(ENABLE_PAPI)
    find_package(PAPI REQUIRED)
    list(APPEND COMPILE_DEFINITIONS WITH_PAPI)
endif()

# Linkage
target_link_libraries(octrees-benchmark
    PUBLIC
        OpenMP::OpenMP_CXX
        Eigen3::Eigen
        ${LASLIB_LIBRARIES}
    PRIVATE
        ${PCL_LIBRARIES}
        ${PAPI_LIBRARIES}
)
```

**Variables de salida:**
- `<Package>_FOUND` - Si encontró la librería
- `<Package>_INCLUDE_DIRS` - Directorio de headers
- `<Package>_LIBRARIES` - Librerías a linkear

---

### 🏗️ `CMakeBuildExec.cmake`
**Configuración de compilación del ejecutable**.

**Contenido:**
```cmake
# Ejecutable principal
add_executable(octrees-benchmark ${SOURCES})

# Directorio include
target_include_directories(octrees-benchmark PRIVATE inc/)

# Flags de compilación
target_compile_options(octrees-benchmark PRIVATE
    -Wall -Wextra -Wpedantic  # Warnings
    -O3 -march=native         # Optimización Release
    -std=c++17                # Standard C++
)

# Definiciones de compilación
target_compile_definitions(octrees-benchmark PRIVATE
    VERSION="1.0"
    ${COMPILE_DEFINITIONS}
)

# Output
set_target_properties(octrees-benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
```

---

### ⚙️ `CMakeConfig.cmake`
**Opciones y configuración global de build**.

**Contenido típico:**
```cmake
# Versión mínima de CMake
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# RPATH para librerías dinámicas
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
```

---

### 📦 `modules/` - Módulos PersonalizadosFindXXX

#### `FindLASLIB.cmake`
Búsqueda personalizada de la librería LASlib.

```cmake
find_path(LASLIB_INCLUDE_DIR NAMES laslib/lasreader.hpp)
find_library(LASLIB_LIBRARY NAMES las)

# Variables generadas
if(LASLIB_INCLUDE_DIR AND LASLIB_LIBRARY)
    set(LASLIB_FOUND TRUE)
    set(LASLIB_INCLUDE_DIRS ${LASLIB_INCLUDE_DIR})
    set(LASLIB_LIBRARIES ${LASLIB_LIBRARY})
endif()

# Crear target (mejor práctica)
if(LASLIB_FOUND AND NOT TARGET LASLIB::LASLIB)
    add_library(LASLIB::LASLIB INTERFACE)
    target_include_directories(LASLIB::LASLIB INTERFACE ${LASLIB_INCLUDE_DIRS})
    target_link_libraries(LASLIB::LASLIB INTERFACE ${LASLIB_LIBRARIES})
endif()
```

#### `FindPCL.cmake`, `FindPapi.cmake`, `FindPicotree.cmake`
Similar para otras librerías opcionales.

---

## Flujo Típico de Build

### 1. **Configuración**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
```
Resultado: Directorio `build/` con Makefile o project files

### 2. **Compilación**
```bash
cmake --build build
# O
cd build && make -j4
```
Resultado: `build/octrees-benchmark` (ejecutable)

### 3. **Tests (opcional)**
```bash
cmake -B build -DBUILD_TESTS=ON .
cmake --build build
ctest --output-on-failure
```

### 4. **Install (opcional)**
```bash
cmake --build build --target install
# O
cmake --install build --prefix /usr/local
```

---

## Variables de CMake Comunes

| Variable | Propósito | Ejemplo |
|----------|-----------|---------|
| `CMAKE_BUILD_TYPE` | Tipo de build | Release, Debug, RelWithDebInfo |
| `CMAKE_CXX_COMPILER` | Compilador C++ | clang++, g++ |
| `CMAKE_CXX_STANDARD` | Versión de C++ | 17, 20 |
| `CMAKE_INSTALL_PREFIX` | Directorio install | /usr/local |
| `BUILD_TESTS` | Compilar tests | ON/OFF |
| `ENABLE_PAPI` | Habilitar PAPI | ON/OFF |

### En línea de comandos
```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DENABLE_PAPI=ON \
    -DCMAKE_CXX_COMPILER=clang++ \
    .
```

---

## Generación de archivos

### compile_commands.json
Para herramientas como clangd:
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
# Resultado: build/compile_commands.json
```

### Documentación Doxygen
Si se configura en CMakeLists.txt:
```bash
cmake --build build --target doxygen
```

---

## Troubleshooting

### Librería no encontrada
```
CMake Error: Could not find LASlib
```

Soluciones:
1. Asegurar que librería está instalada
2. Establecer path manualmente:
   ```bash
   cmake -B build -DLASLIB_ROOT=/path/to/las .
   ```
3. Verificar módulo FindLASLIB.cmake existe

### Incompatibilidad de compilador
```bash
# Usar compilador específico
cmake -B build -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 .
```

### Versión de CMake
```bash
# Actualizar CMake
sudo apt-get install cmake
```

---

## Archivos Generados

Después de `cmake -B build`:
```
build/
├── CMakeCache.txt          # Caché de variables
├── Makefile                # Makefile generado
├── CMakeFiles/             # Archivos internos
├── CMakeLists.txt          # Copia del CMakeLists.txt
└── (compilación posterior generará .o y ejecutable)
```

---

## Testing con CTest

CMake usa CTest para ejecutar tests:

```bash
# Descubrir tests
cd build && ctest --verbose

# Ejecutar solo test específico
ctest -R "OctreeTest" --output-on-failure

# Con múltiples threads
ctest -j 4 --output-on-failure
```

---

## Extensiones Frecuentes

### Agregar nueva dependencia externa
1. Crear `modules/FindXXX.cmake` (si no existe)
2. En `CMakeLibraries.cmake`:
   ```cmake
   find_package(XXX REQUIRED)
   target_link_libraries(octrees-benchmark PUBLIC ${XXX_LIBRARIES})
   ```

### Agregar nuevo archivo fuente
1. Archivos en `src/*.cpp` → incluidos automáticamente
2. O agregar explícitamente en `CMakeSources.cmake`

---

## Notas Finales

- CMake es **declarativo**: describe qué compilar, no cómo
- Generadores múltiples: Makefile, Ninja, Visual Studio, Xcode
- Portable: mismo CMakeLists.txt en Windows/Linux/Mac
- Modular: archivosCMake separados para cada aspecto

