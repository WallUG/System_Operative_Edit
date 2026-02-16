#!/bin/bash
set -e

KERNEL="build/kernel.elf"
ISO_DIR="build/iso"
ISO_FILE="build/system_operative_edit.iso"

mkdir -p "$ISO_DIR/boot/grub"

# Copy kernel
cp "$KERNEL" "$ISO_DIR/boot/"

# Create grub.cfg
cat > "$ISO_DIR/boot/grub/grub.cfg" << EOF
set timeout=0
set default=0

menuentry "System Operative Edit" {
    multiboot /boot/kernel.elf
    boot
}
EOF

# Create ISO
grub-mkrescue -o "$ISO_FILE" "$ISO_DIR"

echo "ISO created: $ISO_FILE"
