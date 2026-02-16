#!/bin/bash
set -e

echo "=== Building System Operative Edit ==="

# Crear directorio build
mkdir -p build
cd build

# Configurar con CMake
cmake ..

# Compilar
make -j$(nproc)

echo "=== Build complete! ==="
echo "Kernel binary: build/kernel.elf"
