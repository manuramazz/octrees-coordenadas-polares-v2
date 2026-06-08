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

ALGOS_RADIUS="neighborsPrune,neighborsStruct"
LOCAL_REORDERS="none,polar,cartesian"
umbral=64


# subset searches
for data in "${datasets_low_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER/full" -e "hilb" --kernels "cube,sphere" -r "2.0,5.0,8.0" -s "all" --sequential --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$umbral"
done

# subset searches
for data in "${datasets_mid_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER/full" -e "hilb" --kernels "cube,sphere" -r "0.5,1.0,3.0" -s "all" --sequential --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "128" --umbral-poda "$umbral"

done

for data in "${datasets_high_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER/full" -e "hilb" --kernels "cube,sphere" -r "0.01,0.05,0.2" -s "all" --sequential --repeats 1 -a "$ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "256" --umbral-poda "$umbral"
done
