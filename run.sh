#!/bin/bash

# Exit on error
set -e

# Define directories
BUILD_DIR="build"
EXEC_NAME="BM25-Multilingual"

# Create build directory if missing
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake if Makefile is missing
if [ ! -f "Makefile" ]; then
    echo "Running CMake..."
    cmake ..
fi

# Compile the project
echo "Compiling project..."
cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

cd ..

# Run the executable
echo "Running $EXEC_NAME..."
echo "-----------------------------------"
./build/"$EXEC_NAME" "$@"
