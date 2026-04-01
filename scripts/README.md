# `/scripts/` - Scripts de Instalación de Dependencias

Directorio que contiene scripts bash para instalar automáticamente las dependencias externas del proyecto.

## Archivos

```
scripts/
├── install_laslib.sh        # Instalar LASlib
├── install_pcl.sh           # Instalar Point Cloud Library (opcional)
├── install_papi.sh          # Instalar PAPI (opcional)
└── install_picotree.sh      # Instalar PicoTree (opcional)
```

---

## 📦 `install_laslib.sh`
**Instala LASlib** - Librería para leer archivos LAS (formato LiDAR estándar).

**Responsabilidades:**
1. Instalar dependencias del sistema (libjpeg, libpng, zlib, etc.)
2. Clonar repositorio LAStools
3. Compilar LASlib
4. Instalar en ubicación estándar

**Proceso:**
```bash
# 1. Dependencias del sistema
sudo apt-get install libjpeg62 libpng-dev libtiff-dev ...

# 2. Clonar LAStools
git clone https://github.com/LAStools/LAStools lib/LAStools

# 3. Compilar
cd lib/LAStools
cmake -B build -DCMAKE_BUILD_TYPE=Release ...
cmake --build build
cmake --install build

# 4. Resultado: LASlib instalada
```

**Ubicación de instalación:**
- Headers: `${CMAKE_PREFIX_PATH}/include/laslib/`
- Librerías: `${CMAKE_PREFIX_PATH}/lib/liblas.*`

**Obligatoria para el proyecto.**

---

## 📦 `install_pcl.sh`
**Instala Point Cloud Library (PCL)** - Librería de procesamiento de nubes de puntos.

**Características:**
- Proporciona octrees y KD-trees alternativos para benchmarking
- Punto de comparación importante
- **Opcional** - El proyecto funciona sin ella

**Proceso:**
```bash
# 1. Descargar PCL source (versión 1.15 o superior)
wget https://github.com/PointCloudLibrary/pcl/releases/download/pcl-1.15.0/source.tar.gz

# 2. Compilar desde source
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/local/pcl ...
make -j2
make install

# 3. Resultado: PCL instalada en ~/local/pcl/
```

**Ubicación esperada:**
- Path: `$HOME/local/pcl/`
- Configurable en `cmake/CMakeLibraries.cmake`

**Ventaja:**
- Habilita comparación con PCL Octree y KD-tree
- Benchmarks más comprehensivos

**Si no se instala:**
- Compilación continúa normalmente
- Benchmarks de PCL simplemente no se ejecutan
- Mensaje de warning en compilación

---

## 📦 `install_papi.sh`
**Instala PAPI (Performance API)** - Herramienta para profiling de hardware.

**Características:**
- Mide eventos de CPU (cache misses, branch mispredictions, etc.)
- **Opcional** - Detalles útiles pero no críticos
- Requiere acceso al contador de rendimiento del kernel

**Proceso:**
```bash
# 1. Descargar PAPI
wget https://github.com/icl-utk-edu/papi/releases/download/papi-7-2-0-t/papi-7.2.0.tar.gz

# 2. Compilar e instalar
tar xvf papi-7.2.0.tar.gz
cd papi-7.2.0/src
./configure --prefix=$(pwd)/..
make
make install

# 3. Resultado: PAPI instalada
```

**Eventos disponibles:**
- `PAPI_L1_DCM` - L1 Data cache misses
- `PAPI_L2_DCM` - L2 cache misses
- `PAPI_LL_DCM` - Last-Level cache misses
- `PAPI_BR_MSP` - Branch mispredictions
- `PAPI_FP_OPS` - Floating-point operations

**Requisitos del sistema:**
- Kernel Linux moderno (con soporte de contadores)
- Acceso a `/proc/sys/kernel/perf_event_paranoid` (puede requerir sudo)

**Activación:**
```bash
cmake -B build -DENABLE_PAPI=ON .
```

---

## 📦 `install_picotree.sh`
**Instala PicoTree** - Estructura de búsqueda espacial alternativa.

**Características:**
- Punto de comparación adicional
- **Opcional**
- Estructura eficiente de KD-tree

**Proceso:**
```bash
# 1. Clonar repositorio
git clone https://github.com/adriansm/picotree lib/picotree

# 2. Copiar headers
cp -r lib/picotree/include/* ${CMAKE_PREFIX_PATH}/include/

# 3. Resultado: PicoTree headers disponibles
```

**Nota:**
- Librería header-only
- No requiere compilación
- Solo copiar archivos

---

## Uso de los Scripts

### Instalar todas las dependencias obligatorias

```bash
cd proyecto_tfg/octrees-coordenadas-polares-v2
./scripts/install_laslib.sh
```

### Instalar dependencias opcionales

```bash
# PCL (comparación con referencia estándar)
./scripts/install_pcl.sh

# PAPI (profiling de caché)
./scripts/install_papi.sh

# PicoTree (otra estructura de comparación)
./scripts/install_picotree.sh
```

### Instalar todo

```bash
./scripts/install_laslib.sh && \
./scripts/install_pcl.sh && \
./scripts/install_papi.sh && \
./scripts/install_picotree.sh
```

---

## Automatización en CI/CD

En GitHub Actions o similar:

```yaml
name: Setup Dependencies
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install LASlib
        run: ./scripts/install_laslib.sh
        
      - name: Install PCL
        run: ./scripts/install_pcl.sh
        
      - name: Install PAPI
        run: ./scripts/install_papi.sh
        
      - name: Build Project
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release .
          cmake --build build
```

---

## Troubleshooting

### LASlib no encuentra dependencias

```bash
sudo apt-get install libjpeg62 libpng-dev libtiff-dev libjpeg-dev \
    libz-dev libproj-dev liblzma-dev libjbig-dev libzstd-dev \
    libgeotiff-dev libwebp-dev libsqlite3-dev
```

### PCL compilación muy lenta

- Normal en máquinas lentas
- Reducir threads: `make -j1` (más lento)
- Usar máquina más rápida en segunda pasada

### PAPI no funciona

```bash
# Verificar permisos
cat /proc/sys/kernel/perf_event_paranoid

# Si > 2, requiere sudo
sudo bash -c 'echo 2 > /proc/sys/kernel/perf_event_paranoid'
```

### Rutas customizadas

Editar scripts para cambiar ubicación de instalación:

```bash
# En install_pcl.sh
CMAKE_INSTALL_PREFIX="/custom/path/pcl"
```

---

## Dependencias del Sistema Requeri das

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install \
    cmake \
    g++ \
    git \
    build-essential \
    libomp-dev \
    (et las dependencias específicas por script)
```

### macOS

```bash
brew install cmake gcc git
# Luego ejecutar scripts
```

### Alternativa: Docker

Si tienes Dockerfile en el proyecto:

```bash
docker build -t octrees-benchmark .
docker run -it octrees-benchmark bash
```

---

## Verificación de Instalación

Después de instalar, verificar que funciona:

```bash
# Compilar proyecto
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build

# Si falta alguna dependencia, el error de CMake lo indicará
```

---

## Notas

- Scripts son **Bash** (compatible con Linux/WSL/macOS)
- Requieren **sudo** para instalar en ubicaciones del sistema
- Pueden ejecutarse múltiples veces sin problemas
- Ideal para entornos de desarrollo limpio

