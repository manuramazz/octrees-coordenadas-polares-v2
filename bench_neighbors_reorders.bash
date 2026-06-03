# setup
FOLDER="out_tfg_reorders_v2"
set -e
mkdir -p "$FOLDER"

# data/hessigheim/Mar18_train.las
# data/hessigheim/Mar18_test.las
# data/dales_las/5085_54320.las (14.4 M)
# data/dales_las/5095_54440.las (14.4 M)

# datasets
datasets_low_density=(
    "data/dales_las/5085_54320.las"
    "data/dales_las/5095_54440.las"
    "data/pnoa/PNOA_2024_PNR_489-4672_NPC01.las"
)

datasets_mid_density=(
    "data/paris_lille/Lille_0.las"
    "data/paris_lille/Paris_Luxembourg_6.las"
)
datasets_high_density=(
    "data/semantic3d/bildstein_station1_xyz_intensity_rgb.las" 
    "data/semantic3d/sg27_station8_intensity_rgb.las"
)

MAX_POINTS_LEAF=(128, 256, 512, 768, 1024)
UMBRALES_PODA=(16, 64, 100)
N_SEARCHES="10000"
FULL_OURS="neighborsPrune"
FULL_ALGOS_RADIUS="neighborsPrune"
LOCAL_REORDERS="none,polar,cartesian"


# subset searches
for data in "${datasets_low_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" --kernels "cube,sphere" -r "1.0,2.0,3.0,5.0,6.0,7.0,8.0,10.0" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done

# subset searches
for data in "${datasets_mid_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" --kernels "cube,sphere" -r "0.25,0.5,0.75,1.0,1.5,2.0,3.0,5.0" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done

for data in "${datasets_high_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" --kernels "cube,sphere" -r "0.005,0.01,0.025,0.05,0.1,0.15,0.2,0.3" -s "$N_SEARCHES" --repeats 5 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done
