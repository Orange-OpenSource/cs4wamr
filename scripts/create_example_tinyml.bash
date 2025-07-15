
PROJECT_BASE="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )/..")"

if [ -z "$TVM_HOME" ]; then
source "${PROJECT_BASE}/scripts/get_RIOT_ML_variables.bash"
fi

echo PROJECT_BASE "$PROJECT_BASE"
DEFAULT_WASM_FILES="$PROJECT_BASE/examples/tinyml/RIOT-to-Wasm-TinyML"
RIOT_ML_PATH="$PROJECT_BASE/dependencies/RIOT-ML"

OUTPUT_DIR="${1:-./model}"
OUTPUT_DIR="$(readlink -f -- "$OUTPUT_DIR")"
BOARD="${2:-arduino-nano-33-ble}"
MODEL=$RIOT_ML_PATH/model_zoo/ds_cnn_s_quantized.tflite

echo "Creating model for $BOARD in $OUTPUT_DIR" 
# shellcheck disable=SC2015
(cd "$RIOT_ML_PATH" && python3 export_standalone.py --board "$BOARD" "$MODEL" "$OUTPUT_DIR" || { echo "Failed to export model"; exit 1; })
rm "$OUTPUT_DIR/main.c" "$OUTPUT_DIR/Makefile"
mkdir -p "$OUTPUT_DIR"
cp -r "$DEFAULT_WASM_FILES/"* "$OUTPUT_DIR"

export PROJECT_BASE
export DEFAULT_WASM
export OUTPUT_DIR
export BOARD
export MODEL