# setup
FOLDER="prueba"
set -e
mkdir -p "$FOLDER"

# data/hessigheim/Mar18_train.las
# data/hessigheim/Mar18_test.las
# data/dales_las/5085_54320.las (14.4 M)
# data/dales_las/5095_54440.las (14.4 M)

# datasets
datasets_prueba=(
    "data/hessigheim/Mar18_train.las"
    "data/hessigheim/Mar18_test.las"
)


MAX_POINTS_LEAF=(256)
UMBRALES_PODA=(64)
N_SEARCHES="10000"
FULL_OURS="neighborsPrune"
FULL_ALGOS_RADIUS="neighborsPrune"
LOCAL_REORDERS="none,polar,cartesian"


# subset searches
for data in "${datasets_prueba[@]}"; do
  for leaf in "${MAX_POINTS_LEAF[@]}"; do
    for umbral in "${UMBRALES_PODA[@]}"; do
      ./build/octrees-benchmark -i "$data" -o "$FOLDER" --kernels "cube,sphere" -r "0.1, 0.5, 2.0" -s "$N_SEARCHES" --repeats 1 -a "$FULL_ALGOS_RADIUS" --local-reorder "$LOCAL_REORDERS" --max-leaf "$leaf" --umbral-poda "$umbral"
    done
  done
done

