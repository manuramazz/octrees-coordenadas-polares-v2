# Manual de usuario — octrees-benchmark-coordenadas-polares

Este manual documenta el uso del programa `octrees-benchmark-coordenadas-polares`, que parte de la implementación base del Octree lineal desarrollada por Viñambres et al. El presente trabajo se centra en el diseño e implementación de las técnicas de poda a nivel de nodo hoja sobre las estructuras de datos existentes (Octrees) y sus correspondientes algoritmos de búsqueda.

## Instalación

### Dependencias

#### LASTools

En primer lugar es necesario instalar las dependencias de LASTools, listadas en https://github.com/LAStools/LAStools:

```bash
sudo apt-get install libjpeg62 libpng-dev libtiff-dev libjpeg-dev \
    libz-dev libproj-dev liblzma-dev libjbig-dev libzstd-dev \
    libgeotiff-dev libwebp-dev liblzma-dev libsqlite3-dev
```

A continuación, se clona el repositorio y se compila:

```bash
git clone --depth 1 https://github.com/LAStools/LAStools lib/LAStools
(cd lib/LAStools && cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$PWD/../LASlib . && \
    cmake --build build -- -j && cmake --install build)
rm -rf lib/LAStools
```

#### PCL versión 1.15 (Opcional)

Se debe obtener el código fuente de la versión 1.15 desde https://github.com/PointCloudLibrary/pcl/releases y compilarlo. La carpeta de instalación usada por defecto es `~/local/pcl`, aunque puede modificarse, junto con la versión buscada, editando el fichero `CMakeLibraries.cmake`.

```bash
wget https://github.com/PointCloudLibrary/pcl/releases/download/pcl-1.15.0/source.tar.gz
tar xvf source.tar.gz
cd pcl && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/local/pcl -DCMAKE_BUILD_TYPE=Release
make -j2
make -j2 install
```

Si PCL no se encuentra durante la compilación, el proyecto se compilará igualmente con normalidad, pero sin soporte para los algoritmos de benchmark basados en el Octree y el KD-Tree de PCL.

#### PAPI (Opcional, para perfilado de caché)

```bash
wget https://github.com/icl-utk-edu/papi/releases/download/papi-7-2-0-t/papi-7.2.0.tar.gz -P lib
tar xvf lib/papi-7.2.0.tar.gz -C lib && rm lib/papi-7.2.0.tar.gz \
    && mv lib/papi-7.2.0 lib/papi/
(cd lib/papi/src && ./configure --prefix=$(pwd)/.. && make -j \
    && make install)
```

### Compilación

Una vez instaladas las dependencias necesarias, dentro del directorio del proyecto basta con ejecutar:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release .
cmake --build build
```

Esto genera el ejecutable en `build/octrees-benchmark`.

Para mayor comodidad, existe el script de bash `compile.sh`, que ejecuta esos comandos además de limpiar el directorio de build (recomendado borrar el directorio siempre que se compile después de introducir cambios en el código).

### Verificación de la instalación: Tests

El proyecto incluye una batería de tests unitarios mediante GoogleTest, que permite comprobar que la instalación y compilación se realizaron correctamente. Para habilitarla, se debe compilar el proyecto con la opción `-DBUILD_TESTS=ON`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON .
cmake --build build
```

Tras compilar el proyecto, todos los tests se pueden ejecutar mediante:

```bash
make                # compila la biblioteca y todos los ejecutables de test
ctest --output-on-failure
```

También es posible ejecutar cada batería de tests de forma individual:

```bash
./tests/test_points
./tests/test_encoders
./tests/test_octree
./tests/test_octree_advanced
```

Para ejecutar un caso de test concreto con CTest, se utiliza la opción `-R` seguida del nombre del test:

```bash
ctest -R LinearOctreeTest.RadiusSearch
```

Adicionalmente, bajo `tests/example.cpp` se incluye un pequeño ejemplo de uso de las funcionalidades de la biblioteca, que se compila automáticamente junto con el proyecto principal:

```bash
make test_library
./tests/test_library
```

## Ejecución

El programa se ejecuta a través del binario generado en la compilación, `build/octrees-benchmark`, configurando su comportamiento mediante parámetros de línea de comandos. A continuación se muestra un ejemplo de invocación, correspondiente a uno de los experimentos descritos en la memoria: una búsqueda completa secuencial de vecinos sobre la nube `Paris_Luxembourg_6`, comparando los distintos reordenamientos existentes sobre los dos algoritmos de búsqueda sobre los que se trabajó para una selección de radios.

```bash
./build/octrees-benchmark --kernels "sphere" -i "data/paris_lille/Lille_0.las" \
    -o "out_tfg_reorders_v3/full" -e "hilb" --kernels "cube,sphere" \
    -r "2.0,5.0,8.0" -s "all" --sequential --repeats 1 \
    -a "neighborsPrune,neighborsStruct" \
    --local-reorder "none,polar,cartesian" --max-leaf "256" --umbral-poda "64"
```

Para reproducir el conjunto completo de experimentos descritos en los capítulos de Pruebas y Discusión de Resultados de la memoria, el proyecto incluye una serie de scripts:

- `bench_neighbors_reorders.bash`: búsquedas de vecinos de centros aleatorios.
- `bench_neighbors_full_reorders.bash`: búsquedas de vecinos de coberturas completas.
- `bench_neighbors_parallel.bash`: búsquedas de vecinos para diferentes escalas de paralelización, tanto de centros aleatorios como coberturas completas.
- `bench_neighbors_debug_range_selector.bash`: búsquedas reducidas (60 búsquedas) sobre el mismo conjunto de nubes, con la opción de depuración de podas habilitada, devolviendo archivos con el número de puntos podados por hoja. **Modificar con precaución**: generan archivos muy pesados, escribiendo una línea por hoja evaluada en cada búsqueda.
- Scripts del estudio de partida: `bench_locality.bash`, `bench_memory.bash`, `bench_neighbors.bash`, `bench_neighbors_temp.bash`.

Asimismo, bajo la carpeta `plots_reorders` se incluyen todos los scripts y notebooks en Python empleados para generar las figuras presentadas en la memoria. Se puede consultar cada gráfica para cada una de las nubes de puntos estudiadas, ya que no todas fueron incluidas en la memoria. En la carpeta `plots` se encuentran las figuras de los resultados publicados por Viñambres et al., también disponibles para consulta.

### Ejecución en el CESGA (FinisTerrae III)

Para ejecutar el programa bajo el entorno hardware del supercomputador CESGA es necesario realizar una serie de ajustes respecto al proceso de instalación local descrito en las secciones anteriores.

#### Carga de módulos

En el CESGA las dependencias del sistema se gestionan mediante módulos. Antes de compilar es necesario cargar la versión correcta del compilador GCC y la biblioteca PAPI ejecutando:

```bash
module purge
module load gcc/12.3.0
module load papi
```

#### Instalación de LASTools

La dependencia LASTools no está disponible como módulo precargado, por lo que debe instalarse manualmente a partir de su repositorio siguiendo el mismo procedimiento descrito en la sección de [Dependencias](#dependencias).

#### Ajuste del fichero de configuración de CMake

Para que CMake localice correctamente la biblioteca LASTools en el entorno del CESGA es necesario sustituir el fichero `cmake/modules/FindLASLIB.cmake` del proyecto por la versión modificada incluida en el repositorio bajo el nombre `FindLASLIB_cesga.cmake`. Basta con sobrescribir el fichero original:

```bash
cp FindLASLIB_cesga.cmake cmake/modules/FindLASLIB.cmake
```

Este fichero modificado especifica explícitamente las rutas de búsqueda de cabeceras y biblioteca de LASTools compatibles con el entorno del CESGA, ya que las rutas por defecto no son válidas en este sistema.

#### Compilación

Una vez realizados los ajustes anteriores, la compilación se lleva a cabo con:

```bash
cmake -DCMAKE_C_COMPILER=gcc          \
      -DCMAKE_CXX_COMPILER=g++        \
      -DCMAKE_BUILD_TYPE=Release      \
      -DCMAKE_PREFIX_PATH=$HOME/my_libs/LAStools \
      ..
cmake --build . -j2
```

Para mayor comodidad, estos pasos están automatizados en el script `cesga_compile.sh` incluido en el repositorio. Basta con ejecutarlo desde el directorio raíz del proyecto para compilar correctamente el programa en un nodo de computación del FinisTerrae III.

#### Ejecución mediante SLURM

El CESGA utiliza el gestor de colas SLURM para la ejecución de trabajos. Se incluyen scripts con configuraciones de jobs que solicitan los recursos necesarios para cada script de benchmark:

| Script de envío | Script que ejecuta |
|---|---|
| `job_submit` | `bench_neighbors_reorders.bash` |
| `job_full_submit` | `bench_neighbors_full_reorders.bash` |
| `job_parallel_submit` | `bench_neighbors_parallel.bash` |
| `ranges_submit` | `bench_neighbors_debug_range_selector.bash` |

Estos scripts no compilan el programa; se debe compilar manualmente de forma previa y luego enviar los jobs a la cola de SLURM con el comando `sbatch`:

```bash
sbatch job_submit
```

### Formato de entrada

El programa toma como entrada nubes de puntos en formato binario `.LAS`, gestionadas internamente mediante la biblioteca LASTools.

### Formato de salida

Los resultados de cada ejecución se almacenan en la ruta indicada mediante el parámetro `-o`/`--output`. El programa genera ficheros con las métricas temporales y de efectividad de poda correspondientes a la configuración de parámetros especificada, que sirven de entrada a los scripts de generación de gráficas.

### Verificación del funcionamiento de las podas

En la ruta `test_funcionalidad_podas/test_vecindades.ipynb` está el test de Python que comprueba si los resultados usando reordenamientos son idénticos a los resultados sin hacer uso de la optimización (función `verificar_integridad_vecinos`). Los parámetros a introducir son los siguientes:

- **data_path**: ruta de los archivos `.csv` devueltos por las ejecuciones del programa.
- **clouds**: el test comprobará todos los casos posibles solamente para las nubes aquí especificadas.

## Parámetros del programa

A continuación se detallan todos los parámetros aceptados por el programa. Para consultar esta misma información desde la propia terminal:

```bash
./build/octrees-benchmark --help
```

| Opción | Alias | Descripción |
|:---|:---|:---|
| `-h` | `--help` | Muestra el mensaje de ayuda. |
| `-i` | `--input` | Ruta del fichero de entrada. |
| `-c` | `--container-type` | Tipo de contenedor a utilizar. Por defecto: `AoS`. Valores posibles: `SoA`, `AoS`. |
| `-o` | `--output` | Ruta del fichero de salida. |
| `-r` | `--radii` | Radios de búsqueda a evaluar (separados por comas, p. ej. `2.5,5.0,7.5`). |
| `-v` | `--kvalues` | Valores de k para las búsquedas KNN (separados por comas, p. ej. `10,50,250,1000`). |
| `-s` | `--searches` | Número de búsquedas a realizar (con centros aleatorios, salvo que se indique `--sequential`). Usar `all` para buscar sobre la totalidad de la nube. |
| `-t` | `--repeats` | Número de repeticiones a realizar para cada configuración del benchmark. |
| `-k` | `--kernels` | Kernels de búsqueda a utilizar (separados por comas o `all`). Valores posibles: `sphere`, `cube`, `square`, `circle`. |
| `-a` | `--search-algo` | Algoritmos de búsqueda a ejecutar (separados por comas o `all`). Por defecto: `neighborsPtr,neighbors,neighborsPrune,neighborsStruct`. Ver tabla de algoritmos más abajo. |
| `-e` | `--encodings` | Codificaciones SFC con las que reordenar la nube (separadas por comas o `all`). Por defecto: `all`. Valores: `none`, `mort` (Morton), `hilb` (Hilbert). |
| `--` | `--local-reorders` | Reordenamientos locales a nivel de hoja (separados por comas o `all`). Por defecto: `all`. Valores: `none`, `polar` (ángulo azimutal φ), `cartesian` (coordenadas x, y, z). |
| `-u` | `--umbral-poda` | Umbral mínimo de puntos por hoja para activar el selector de rango y la poda. Por defecto: 16. Hojas con menos puntos se recorren secuencialmente. |
| `--` | `--debug-ranges` | Activa el registro detallado de la selección de rangos en cada hoja visitada durante las búsquedas. |
| `--` | `--debug` | Activa el modo de depuración general (mide tiempos de construcción del Octree y de codificación). |
| `--` | `--build-enc` | Ejecuta benchmarks para la codificación y construcción de las estructuras seleccionadas. |
| `--` | `--memory` | Benchmark de memoria consumida por una estructura. Valores: `ptrOct`, `linOct`, `unibnOct`, `nanoKD`, `pclOct`, `pclKD`, `picoTree`. |
| `--` | `--locality` | Benchmarks para analizar la localidad de la nube tras los reordenamientos. |
| `--` | `--cache-profiling` | Activa el perfilado de caché durante las búsquedas mediante PAPI. |
| `--` | `--check` | Activa la comprobación de resultados (opción heredada; se recomienda usar `avg_result_size`). |
| `--` | `--no-warmup` | Desactiva la fase de calentamiento previa a las mediciones. |
| `--` | `--approx-tol` | Tolerancias para la búsqueda aproximada (separadas por comas, p. ej. `10.0,50.0,100.0`). |
| `--` | `--num-threads` | Número de hilos para el test de escalabilidad (separados por comas, p. ej. `1,2,4,8,16,32`). Si no se especifica, OpenMP usa el máximo disponible. |
| `--` | `--sequential` | Hace las búsquedas secuenciales en lugar de aleatorias. Se activa automáticamente con `-s all`. |
| `--` | `--max-leaf` | Número máximo de puntos por hoja del Octree (por defecto = 128). No aplica al Octree de PCL. |
| `--` | `--pcl-oct-resolution` | Tamaño mínimo de octante para la subdivisión en el Octree de PCL. |

### Algoritmos de búsqueda (`--search-algo`)

**Búsqueda por radio:**

| Valor | Descripción |
|:---|:---|
| `neighborsPtr` | Búsqueda básica sobre el Octree de punteros. |
| `neighbors` | Búsqueda básica sobre el Octree lineal. |
| `neighborsPrune` | Búsqueda optimizada sobre el Octree lineal con poda de octantes. |
| `neighborsStruct` | Búsqueda optimizada mediante rangos de índices. |
| `neighborsApprox` | Búsqueda aproximada (cotas superior/inferior), requiere `--approx-tol`. |
| `neighborsUnibn` | Búsqueda mediante unibnOctree. |
| `neighborsPCLKD` | Búsqueda mediante el KD-Tree de PCL (si está disponible). |
| `neighborsPCLOct` | Búsqueda mediante el Octree de PCL (si está disponible). |
| `neighborsPico` | Búsqueda mediante PicoTree. |

**Búsqueda KNN:**

| Valor | Descripción |
|:---|:---|
| `KNNV2` | Búsquedas KNN sobre el Octree lineal. |
| `KNNNanoflann` | Búsquedas KNN mediante nanoflann. |
| `KNNPCLKD` | Búsqueda KNN mediante el KD-Tree de PCL (si está disponible). |
| `KNNPCLOCT` | Búsqueda KNN mediante el Octree de PCL (si está disponible). |
| `KNNPico` | Búsqueda KNN mediante PicoTree. |

## Mensajes de error y problemas comunes

- **El programa no encuentra el fichero de entrada**: comprobar que la ruta indicada en `-i`/`--input` es correcta y que el fichero existe, tiene formato `.LAS` y tiene permisos de lectura.

- **Hay dependencias no enlazadas**: posible error en el proceso de compilación. Comprobar qué dependencia está provocando el fallo y revisar los pasos de instalación de este manual. Nota: la ausencia del módulo PCL lanzará un aviso pero no detendrá la compilación del programa.

- **Se configuró una combinación de algoritmo y reordenamiento no soportada**: comprobar los parámetros de entrada. Si los algoritmos son distintos a `neighborsPrune` o `neighborsStruct`, el programa se ejecutará ignorando la opción `--local-reorders`. Si se combina `neighborsStruct` con `cartesian`, se mostrará un mensaje indicando que esa combinación no está soportada. Lo mismo ocurre al activar `--debug-ranges` junto con `neighborsStruct`.

## Autoría
Trabajo de Fin de Grado realizado por 
- Manuel Ramallo Blanco (manuel.ramallo@rai.usc.es)

Tutoría a cargo de
- Miguel Yermo García (miguel.yermo@usc.es)
- Francisco M. Fernández Rivera (ff.rivera@usc.es)

Implementación del Octree lineal original integrando aplicación de curvas de llenado de espacio:
- https://github.com/derivada/octrees-benchmark