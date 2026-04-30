# setup
FOLDER="outv1.2"
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

N_SEARCHES="5000"
ALGO="neighborsPrune"

# range debug searches (subsets)
for data in "${datasets_low_density[@]}"; do 
  ./build/octrees-benchmark -i "$data" -o "$FOLDER" --kernels "cube,sphere" -r "0.5,1.0,2.0,3.0" -s "$N_SEARCHES" --repeats 3 -a "$ALGO" --local-reorder "none,spherical,cylindrical" --num-threads 1 --debug-leaves-time
done
for data in "${datasets_high_density[@]}"; do
  ./build/octrees-benchmark -i "$data" -o "$FOLDER" --kernels "cube,sphere" -r "0.01,0.05,0.1,0.2" -s "$N_SEARCHES" --repeats 3 -a "$ALGO" --local-reorder "none,spherical,cylindrical" --num-threads 1 --debug-leaves-time
done