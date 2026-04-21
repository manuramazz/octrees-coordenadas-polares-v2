#!/bin/bash

# --- Configuración de Slurm ---
#SBATCH -p compute           # Partición (cola) de computación general
#SBATCH -n 1                 # Un solo "task" (tu script de bash)
#SBATCH -c 40                # Reservar 40 núcleos (coincide con tu THREADS máximo)
#SBATCH --mem=64G            # Memoria RAM (ajusta según el peso de Semantic3D)
#SBATCH -t 08:00:00          # Tiempo máximo (HH:MM:SS) - sube a 12h si el full search es lento
#SBATCH -J octree_bench_v1      # Nombre del trabajo
#SBATCH -o logs/bench_%j.out # Archivo de salida (crea la carpeta logs antes)
#SBATCH -e logs/bench_%j.err # Archivo de errores
#SBATCH --mail-type=END,FAIL # Notificar al finalizar o si falla
#SBATCH --mail-user=manuel.ramallo@rai.usc.es

module purge
module load gcc/11.2.0
module load cmake/3.21.1
module load numactl

chmod +x compile.sh
chmod +x bench_neighbors_reorders.bash

echo "Iniciando benchmark en el nodo: $SLURMD_NODENAME"
echo "Fecha: $(date)"

bash compile.sh
bash bench_neighbors_reorders.bash

echo "Benchmark finalizado: $(date)"