#!/bin/bash

ISO="build/system_operative_edit.iso"
KERNEL="build/kernel.elf"

echo "=== Running in QEMU ==="

# Si existe la ISO, arranca con ella (via GRUB, magic correcto)
if [ -f "$ISO" ]; then
    echo "Usando ISO: $ISO"
    qemu-system-i386 \
        -cdrom "$ISO" \
        -boot d \
        -m 128M \
        -serial stdio \
        -no-reboot \
        -display sdl

# Si no hay ISO, arranca directo con -kernel (magic de QEMU)
elif [ -f "$KERNEL" ]; then
    echo "Usando kernel directo: $KERNEL (sin GRUB)"
    qemu-system-i386 \
        -kernel "$KERNEL" \
        -m 128M \
        -serial stdio \
        -no-reboot \
        -display sdl
else
    echo "Error: No se encontro ISO ni kernel. Ejecuta build.sh primero."
    exit 1
fi

# Para debug agregar: -s -S -d int,cpu_reset,guest_errors -D qemu.log
