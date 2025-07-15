
PROJECT_BASE="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )/..")"

echo PROJECT_BASE "$PROJECT_BASE"
TVM_HOME="$PROJECT_BASE/dependencies/RIOT-ML/tvm"
TVM_LIBRARY_PATH="$TVM_HOME/build"
PYTHONPATH="$TVM_HOME/python:${PYTHONPATH}"

export PROJECT_BASE
export TVM_HOME
export TVM_LIBRARY_PATH
export PYTHONPATH

# shellcheck disable=SC1091
source "$PROJECT_BASE/.venv/bin/activate"