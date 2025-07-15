CURRENT_DIR="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )")"
PROJECT_BASE="$( readlink -f -- "$CURRENT_DIR/../../../..")"
TARGET_MODEL_DIR="$CURRENT_DIR/../wasm/model"


rm -r "$TARGET_MODEL_DIR" 2> /dev/null || true
source "$PROJECT_BASE/scripts/get_RIOT_ML_variables.bash"
"$PROJECT_BASE/scripts/create_example_tinyml.bash" "$TARGET_MODEL_DIR"
sed -i '102,+33d' $TARGET_MODEL_DIR/mlmci/mlmci_impl.c 