# setup
FOLDER="out_tfg_reorders"
set -e
mkdir -p "$FOLDER"


# datasets
datasets_low_density=(
    "data/paris_lille/Lille_0.las"
    "data/dales_las/5145_54340.las"
    "data/paris_lille/Paris_Luxembourg_6.las"
    "data/pnoa/PNOA_2024_PNR_489-4672_NPC01.las"
)
datasets_high_density=(
    "data/semantic3d/bildstein_station1_xyz_intensity_rgb.las" 
    "data/semantic3d/sg27_station8_intensity_rgb.las"
)

N_SEARCHES="10000"
ALGO="neighborsPrune"
THREADS="1,2,4,8,16,24,32,40"
LOCAL_REORDERS="none,polar,cartesian"
MAX_POINTS_LEAF=256
UMBRAL=64


# parallel subset searches
for data in "${datasets_low_density[@]}"; do
    numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 5 -a "$ALGO" --num-threads "$THREADS" -r "0.1,0.25,0.5,1.0,2.0" --local-reorder "$LOCAL_REORDERS" --max-leaf "$MAX_POINTS_LEAF" --umbral-poda "$UMBRAL"
done
for data in "${datasets_high_density[@]}"; do
   numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 5 -a "$ALGO" --num-threads "$THREADS" -r "0.01,0.05,0.1,0.2" --local-reorder "$LOCAL_REORDERS" --max-leaf "$MAX_POINTS_LEAF" --umbral-poda "$UMBRAL"
done