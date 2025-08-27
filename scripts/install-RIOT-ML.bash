

PROJECT_BASE="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )/..")"
echo PROJECT_BASE "$PROJECT_BASE"

TVM_HOME="$PROJECT_BASE/dependencies/RIOT-ML/tvm"
python3 -m venv "$PROJECT_BASE/.venv"
# shellcheck disable=SC1091
source "$PROJECT_BASE/.venv/bin/activate"
pip3 install setuptools Cython numpy tflite decorator psutil packaging attrs typing_extensions

(cd "$PROJECT_BASE/dependencies/RIOT-ML" && git clone --branch v0.18.0 --recursive https://github.com/apache/tvm tvm)

{
    echo "set(CMAKE_BUILD_TYPE RelWithDebInfo)" 
    # LLVM is a must dependency for compiler end
    echo "set(USE_LLVM \"llvm-config --ignore-libllvm --link-static\")"
    echo "set(HIDE_PRIVATE_SYMBOLS ON)"
    # GPU SDKs, turn on if needed
    echo "set(USE_CUDA   OFF)"
    echo "set(USE_METAL  OFF)"
    echo "set(USE_VULKAN OFF)"
    echo "set(USE_OPENCL OFF)"

    # cuBLAS, cuDNN, cutlass support, turn on if needed
    echo "set(USE_CUBLAS OFF)"
    echo "set(USE_CUDNN  OFF)"
    echo "set(USE_CUTLASS OFF)"

    # To verify !!!
    echo "set(USE_GRAPH_EXECUTOR ON)"
    echo "set(USE_PROFILER ON)"
    echo "set(USE_MICRO ON)"
} >> "$TVM_HOME/cmake/config.cmake"

(cd "$TVM_HOME" && rm -rf build && mkdir build && cp ./cmake/config.cmake ./build)
(cd "$TVM_HOME/build" && cmake .. && cmake --build . --parallel "$(nproc)")

TVM_LIBRARY_PATH="$TVM_HOME/build"
PYTHONPATH="$TVM_HOME/python:${PYTHONPATH}"
export TVM_LIBRARY_PATH
export PYTHONPATH
#pip3 install -e "$TVM_HOME/python"
python -c "import tvm; print(tvm.__file__)"


export PROJECT_BASE
export TVM_HOME