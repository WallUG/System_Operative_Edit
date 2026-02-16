#!/bin/bash
set -e

echo "=== Building System Operative Edit ==="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Compile
make -j$(nproc)

echo "=== Build complete! ==="
echo "Kernel binary: build/kernel.elf"
