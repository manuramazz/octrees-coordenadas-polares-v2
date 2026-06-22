#!/bin/bash

# --- Configuración de Slurm ---
#SBATCH -p compute           # Partición
#SBATCH -n 1                 # Tasks
#SBATCH -c 40                # Cores
#SBATCH --mem=20G            # Memoria RAM
#SBATCH -t 10:00:00          # Tiempo máximo
#SBATCH -J out_tfg_reorders_v3    # Nombre del trabajo
#SBATCH -o logs/bench_%j.out  # Archivo de salida (crea la carpeta logs antes)
#SBATCH -e logs/bench_%j.err # Archivo de errores
#SBATCH --mail-type=END,FAIL # Notificar al finalizar o si falla
#SBATCH --mail-user=example@rai.usc.es

module purge
module load gcc/12.3.0
module load papi

chmod +x bench_neighbors_reorders.bash

echo "Iniciando benchmark"
echo "Fecha: $(date)"

bash bench_neighbors_reorders.bash

echo "Benchmark finalizado: $(date)"
echo "l"
