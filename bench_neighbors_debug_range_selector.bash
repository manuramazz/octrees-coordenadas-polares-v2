# setup
FOLDER="out_tfg_ranges"
set -e
mkdir -p "$FOLDER"

# datasets
datasets_low_density=(
    "data/dales_las/5085_54320.las"
    "data/dales_las/5095_54440.las"
    "data/pnoa/PNOA_2024_PNR_489-4672_NPC01.las"
)

datasets_mid_density=(
    "data/hessigheim/Mar18_train.las"
    "data/hessigheim/Mar18_test.las"
    "data/paris_lille/Lille_0.las"
    "data/paris_lille/Paris_Luxembourg_6.las"
)
datasets_high_density=(
    "data/semantic3d/bildstein_station1_xyz_intensity_rgb.las" 
    "data/semantic3d/sg27_station8_intensity_rgb.las"
)

N_SEARCHES="60"
ALGOS_RADIUS="neighborsPrune"

# range debug searches (subsets)
for data in "${datasets_low_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER" -e "hilb" --kernels "cube,sphere" -r "1.0,2.0,3.0,5.0,6.0,7.0,8.0,10.0" -s "$N_SEARCHES" --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --debug-ranges --num-threads 1 
done

# subset searches
for data in "${datasets_mid_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER" -e "hilb" --kernels "cube,sphere" -r "0.25,0.5,0.75,1.0,1.5,2.0,3.0,5.0" -s "$N_SEARCHES" --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --debug-ranges --num-threads 1
done

for data in "${datasets_high_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER" -e "hilb" --kernels "cube,sphere" -r "0.005,0.01,0.025,0.05,0.1,0.15,0.2,0.3" -s "$N_SEARCHES" --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --debug-ranges --num-threads 1
done