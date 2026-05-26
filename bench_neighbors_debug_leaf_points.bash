# setup
FOLDER="outv6-leaves"
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
    "data/pnoa/PNOA_2024_PNR_489-4672_NPC01.las"
    #"data/speulderbos/Speulderbos_2017_TLS.las"
)
MAX_POINTS_LEAF=(128, 256, 512)
UMBRALES_PODA=(32)
N_SEARCHES="5000"
ALGO="neighborsPrune"

# range debug searches (subsets)
for data in "${datasets_low_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER" -e "hilb" --kernels "cube,sphere" -r "0.1,0.3,0.5,1.0,2.0,3.0" -s "$N_SEARCHES" --repeats 1 -a "$ALGO" --local-reorder "none,polar,cartesian" --num-threads 1 --debug-leaves-time --max-leaf "$leaf" --umbral-poda "$umbral" 
    done
  done
done

for data in "${datasets_high_density[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER" -e "hilb" --kernels "cube,sphere" -r "0.01,0.05,0.1,0.2,0.3" -s "$N_SEARCHES" --repeats 1 -a "$ALGO" --local-reorder "none,polar,cartesian" --num-threads 1 --debug-leaves-time --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done