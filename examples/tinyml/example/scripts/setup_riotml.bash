CURRENT_DIR="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )")"
PROJECT_BASE="$( readlink -f -- "$CURRENT_DIR/../../../..")"

"$PROJECT_BASE/scripts/install-RIOT-ML.bash"