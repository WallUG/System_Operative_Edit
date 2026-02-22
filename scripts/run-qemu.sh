#!/bin/bash

KERNEL="build/kernel.elf"

if [ ! -f "$KERNEL" ]; then
    echo "Error: Kernel not found. Run build.sh first."
    exit 1
fi

echo "=== Running in QEMU ==="
qemu-system-i386 \
    -kernel "$KERNEL" \
    -m 128M \
    -serial file:serial.log \
    -display gtk \
    -no-reboot

# Para depuración también se puede añadir: -s -S
