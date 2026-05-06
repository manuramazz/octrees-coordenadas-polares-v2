#!/bin/bash

# --- Configuración de Slurm ---
#SBATCH -p compute           # Partición (cola) de computación general
#SBATCH -n 1                 # Un solo "task" (tu script de bash)
#SBATCH -c 2                # Reservar 40 núcleos (coincide con tu THREADS máximo)
#SBATCH --mem=50G            # Memoria RAM (ajusta según el peso de Semantic3D)
#SBATCH -t 01:00:00          # Tiempo máximo (HH:MM:SS) - sube a 12h si el full search es lento
#SBATCH -J debug_octree_bench_v2.2    # Nombre del trabajo
#SBATCH -o logs/bench_%j.out # Archivo de salida (crea la carpeta logs antes)
#SBATCH -e logs/bench_%j.err # Archivo de errores
#SBATCH --mail-type=END,FAIL # Notificar al finalizar o si falla
#SBATCH --mail-user=manuel.ramallo@rai.usc.es

module purge
module load gcc/12.3.0
module load papi

chmod +x cesga_compile.sh
chmod +x bench_neighbors_debug_range_selector.bash
chmod +x bench_neighbors_debug_leaf_points.bash


echo "Iniciando benchmark"
echo "Fecha: $(date)"

bash cesga_compile.sh
# bash bench_neighbors_debug_range_selector.bash
bash bench_neighbors_debug_leaf_points.bash

echo "Benchmark finalizado: $(date)"
