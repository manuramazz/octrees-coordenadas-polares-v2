# setup
FOLDER="out/resultsv3"
set -e
mkdir -p "$FOLDER"

# datasets
datasets_low_density=(
    "data/paris_lille/Lille_0.las"
    #"data/dales_las/test/5080_54400.las"
    "data/paris_lille/Paris_Luxembourg_6.las"
)
datasets_high_density=(
    "data/semantic3d/bildstein_station1_xyz_intensity_rgb.las" 
    "data/semantic3d/sg27_station8_intensity_rgb.las"
    #"data/speulderbos/Speulderbos_2017_TLS.las"
)

MAX_POINTS_LEAF=(128, 256, 512)
UMBRALES_PODA=(8, 16, 32)
N_SEARCHES="50000"
FULL_OURS="neighbors,neighborsPrune,neighborsPtr"
FULL_ALGOS_RADIUS="neighborsPrune"
THREADS="1,2,4,8,16,24,32,40"
LOCAL_REORDERS="none,polar,cartesian"


# subset searches
for data in "${datasets_low_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" --kernels "cube,sphere" -r "0.5,1.0,2.0,3.0" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done

for data in "${datasets_high_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" --kernels "cube,sphere" -r "0.01,0.05,0.1,0.2" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done

# full searches
# ./build/octrees-benchmark --kernels "cube,sphere" -i "data/semantic3d/sg27_station8_intensity_rgb.las" -o "$FOLDER/full" -r "0.01,0.02,0.03,0.05" -s "all" --sequential --repeats 1 --no-warmup -a "neighborsPrune" -e "all" --local-reorder "$LOCAL_REORDERS"


# # parallel subset searches
# for data in "${datasets_low_density[@]}"; do
#     numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --num-threads "$THREADS" -r "0.1,0.25,0.5,1.0,2.0" --local-reorder "$LOCAL_REORDERS"
# done
# for data in "${datasets_high_density[@]}"; do
#    numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "$data" -o "$FOLDER/parallel_subset" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --num-threads "$THREADS" -r "0.01,0.05,0.1,0.2" --local-reorder "$LOCAL_REORDERS"
# done

# # parallel full searches (slow)
# numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "data/paris_lille/Lille_0.las" -o "$FOLDER/parallel_full" -s "all" --sequential --repeats 1 --no-warmup -a "$FULL_ALGOS_RADIUS" --num-threads $THREADS -r "0.1,0.25,0.5,1.0,2.0" --local-reorder "$LOCAL_REORDERS"
# #numactl --interleave=all ./build/octrees-benchmark --kernels "sphere" -i "data/dales_las/test/5080_54400.las" -o "$FOLDER/parallel_full" -s "all" --sequential --repeats 1 --no-warmup -a "$FULL_ALGOS_RADIUS" --num-threads $THREADS -r "0.1,0.25,0.5,1.0,2.0"
