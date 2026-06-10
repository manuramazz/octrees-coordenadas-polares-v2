# setup
FOLDER="out_tfg_reorders_v3"
set -e
mkdir -p "$FOLDER"


# datasets
datasets_low_density=(
    "data/dales_las/5085_54320.las"
    # "data/dales_las/5095_54440.las"
    "data/pnoa/PNOA_2024_PNR_489-4672_NPC01.las"
)

datasets_mid_density=(
    "data/hessigheim/Mar18_train.las"
    # "data/hessigheim/Mar18_test.las"
    # "data/paris_lille/Lille_0.las"
    "data/paris_lille/Paris_Luxembourg_6.las"
)
datasets_high_density=(
    "data/semantic3d/bildstein_station1_xyz_intensity_rgb.las" 
    # "data/semantic3d/sg27_station8_intensity_rgb.las"
)

N_SEARCHES="10000"
ALGO="neighborsPrune,neighborsStruct"
THREADS="1,2,4,8,16,24,32,40"
LOCAL_REORDERS="none,polar,cartesian"
MAX_POINTS_LEAF=256
UMBRAL=64


# parallel subset searches
for data in "${datasets_low_density[@]}"; do
    numactl --interleave=all ./build/octrees-benchmark -e "hilb" --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 3 -a "$ALGO" --num-threads "$THREADS" -r "1.0,2.0,3.0" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$UMBRAL"
done

for data in "${datasets_mid_density[@]}"; do
    numactl --interleave=all ./build/octrees-benchmark -e "hilb" --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 3 -a "$ALGO" --num-threads "$THREADS" -r "0.5,1.0,2.0" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$UMBRAL"
done

for data in "${datasets_high_density[@]}"; do
   numactl --interleave=all ./build/octrees-benchmark -e "hilb" --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 3 -a "$ALGO" --num-threads "$THREADS" -r "0.01,0.05,0.1" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$UMBRAL"
done

# parallel full searches (slow)
numactl --interleave=all ./build/octrees-benchmark -e "hilb" --kernels "sphere" -i "data/paris_lille/Lille_0.las" -o "$FOLDER/parallel_full" -s "all" --sequential --repeats 1 --no-warmup -a "$ALGO" --num-threads $THREADS -r "0.5,1.0,2.0" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$UMBRAL"
numactl --interleave=all ./build/octrees-benchmark -e "hilb" --kernels "sphere" -i "data/dales_las/test/5095_54440.las" -o "$FOLDER/parallel_full" -s "all" --sequential --repeats 1 --no-warmup -a "$ALGO" --num-threads $THREADS -r "1.0,2.0,3.0" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$UMBRAL"