#!/bin/bash

# Exit on error
set -e

# Usage:
#   ./run.sh [args...]     build and run the native executable (unchanged)
#   ./run.sh python        build the Python extension, run tests, do an import check

EXEC_NAME="BM25-Multilingual"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# ---------------------------------------------------------------
# Python mode
# ---------------------------------------------------------------
if [ "$1" == "python" ]; then
    PY_BUILD_DIR="build-python"

    if [ ! -f ".venv/bin/activate" ]; then
        echo "No .venv found. Create it first:"
        echo "  python3 -m venv .venv && source .venv/bin/activate && pip install pybind11 pytest"
        exit 1
    fi
    source .venv/bin/activate

    echo "Configuring Python bindings..."
    cmake -S . -B "$PY_BUILD_DIR" \
        -DBUILD_PYTHON_BINDINGS=ON \
        -DPython_EXECUTABLE="$(which python)" \
        -Dpybind11_DIR="$(python -m pybind11 --cmakedir)"

    echo "Compiling..."
    cmake --build "$PY_BUILD_DIR" --parallel "$JOBS"

    echo "Running Python tests..."
    PYTHONPATH="$PY_BUILD_DIR/python" pytest tests/python -v

    echo "Import check..."
    PYTHONPATH="$PY_BUILD_DIR/python" python -c \
        "from multilingual_search import SearchEngine; print(SearchEngine())"

    if [ "$2" == "-x" ]; then
        PYTHONPATH="$PY_BUILD_DIR/python" python src/python/test.py
    fi
    exit 0

fi

# ---------------------------------------------------------------
# Native mode (original behaviour)
# ---------------------------------------------------------------
BUILD_DIR="build"

# Create build directory if missing
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Run CMake if Makefile is missing
if [ ! -f "Makefile" ]; then
    echo "Running CMake..."
    cmake ..
fi

echo "Compiling project..."
cmake --build . --parallel "$JOBS"

cd ..

echo "Running $EXEC_NAME..."
echo "-----------------------------------"
./build/"$EXEC_NAME" "$@"