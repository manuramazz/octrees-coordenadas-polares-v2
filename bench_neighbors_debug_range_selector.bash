# setup
FOLDER="out/results"
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

N_SEARCHES="1000"
FULL_ALGOS_RADIUS="neighbors,neighborsPrune"
THREADS="1,2,4,8,16,24,32,40"


# subset searches
for data in "${datasets_low_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER/subset" -e "hilb" --kernels "cube,sphere" -r "0.5,1.0,2.0,3.0" -s "$N_SEARCHES" --repeats 1 -a "$FULL_ALGOS_RADIUS" --local-reorder "none,spherical" --num-threads 1
done