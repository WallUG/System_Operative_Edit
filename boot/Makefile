# Makefile for Bootloader
# System_Operative_Edit Project
# 
# Este Makefile compila los boot sectors y el FreeLoader

# Herramientas de compilación
AS = as
CC = gcc
LD = ld
OBJCOPY = objcopy

# Flags para el ensamblador
ASFLAGS = --32

# Flags para el compilador C
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
         -Wall -Wextra -O2 -I./freeldr

# Flags para el linker
LDFLAGS = -m elf_i386 -nostdlib

# Directorios
BOOTSECT_DIR = bootsect
FREELDR_DIR = freeldr
BUILD_DIR = build

# Archivos de boot sectors
BOOTSECTS = $(BUILD_DIR)/fat.bin $(BUILD_DIR)/fat32.bin $(BUILD_DIR)/isoboot.bin

# Archivos objeto de FreeLoader
FREELDR_OBJS = $(BUILD_DIR)/freeldr.o $(BUILD_DIR)/video.o \
               $(BUILD_DIR)/memory.o $(BUILD_DIR)/disk.o \
               $(BUILD_DIR)/string.o

# Target principal
.PHONY: all
all: directories bootsect freeldr

# Crear directorios de build
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)

# ============================================================================
# Boot Sectors
# ============================================================================

.PHONY: bootsect
bootsect: $(BOOTSECTS)

# Compilar fat.S a fat.bin
$(BUILD_DIR)/fat.bin: $(BOOTSECT_DIR)/fat.S
	@echo "Compilando boot sector FAT12/16..."
	$(AS) $(ASFLAGS) -o $(BUILD_DIR)/fat.o $<
	$(LD) $(LDFLAGS) -Ttext 0x7C00 --oformat binary -o $@ $(BUILD_DIR)/fat.o
	@echo "Boot sector FAT12/16 compilado: $@"

# Compilar fat32.S a fat32.bin
$(BUILD_DIR)/fat32.bin: $(BOOTSECT_DIR)/fat32.S
	@echo "Compilando boot sector FAT32..."
	$(AS) $(ASFLAGS) -o $(BUILD_DIR)/fat32.o $<
	$(LD) $(LDFLAGS) -Ttext 0x7C00 --oformat binary -o $@ $(BUILD_DIR)/fat32.o
	@echo "Boot sector FAT32 compilado: $@"

# Compilar isoboot.S a isoboot.bin
$(BUILD_DIR)/isoboot.bin: $(BOOTSECT_DIR)/isoboot.S
	@echo "Compilando boot sector ISO-9660..."
	$(AS) $(ASFLAGS) -o $(BUILD_DIR)/isoboot.o $<
	$(LD) $(LDFLAGS) -Ttext 0x7000 --oformat binary -o $@ $(BUILD_DIR)/isoboot.o
	@echo "Boot sector ISO-9660 compilado: $@"

# ============================================================================
# FreeLoader
# ============================================================================

.PHONY: freeldr
freeldr: $(BUILD_DIR)/freeldr.sys

# Linker script para FreeLoader
$(BUILD_DIR)/freeldr.ld:
	@echo "Creando linker script..."
	@echo "OUTPUT_FORMAT(binary)" > $@
	@echo "ENTRY(_start)" >> $@
	@echo "SECTIONS {" >> $@
	@echo "    . = 0xF800;" >> $@
	@echo "    .text.entry : { *(.text.entry) }" >> $@
	@echo "    .text : { *(.text) }" >> $@
	@echo "    .rodata : { *(.rodata) }" >> $@
	@echo "    .data : { *(.data) }" >> $@
	@echo "    .bss : { *(.bss) *(COMMON) }" >> $@
	@echo "}" >> $@

# Compilar archivos C de FreeLoader
$(BUILD_DIR)/freeldr.o: $(FREELDR_DIR)/freeldr.c
	@echo "Compilando freeldr.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/video.o: $(FREELDR_DIR)/video.c
	@echo "Compilando video.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: $(FREELDR_DIR)/memory.c
	@echo "Compilando memory.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/disk.o: $(FREELDR_DIR)/disk.c
	@echo "Compilando disk.c..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(FREELDR_DIR)/string.c
	@echo "Compilando string.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Enlazar FreeLoader
$(BUILD_DIR)/freeldr.sys: $(FREELDR_OBJS) $(BUILD_DIR)/freeldr.ld
	@echo "Enlazando FreeLoader..."
	$(LD) $(LDFLAGS) -T $(BUILD_DIR)/freeldr.ld -o $@ $(FREELDR_OBJS)
	@echo "FreeLoader compilado: $@"
	@ls -lh $@

# ============================================================================
# Limpieza
# ============================================================================

.PHONY: clean
clean:
	@echo "Limpiando archivos generados..."
	rm -rf $(BUILD_DIR)
	@echo "Limpieza completada."

# ============================================================================
# Ayuda
# ============================================================================

.PHONY: help
help:
	@echo "Makefile para Bootloader - System_Operative_Edit"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all          - Compila todo (boot sectors + FreeLoader)"
	@echo "  bootsect     - Compila solo los boot sectors"
	@echo "  freeldr      - Compila solo FreeLoader"
	@echo "  clean        - Limpia archivos generados"
	@echo "  help         - Muestra esta ayuda"
	@echo ""
	@echo "Archivos generados:"
	@echo "  build/fat.bin      - Boot sector FAT12/16 (512 bytes)"
	@echo "  build/fat32.bin    - Boot sector FAT32 (512 bytes)"
	@echo "  build/isoboot.bin  - Boot sector ISO-9660 (2048 bytes)"
	@echo "  build/freeldr.sys  - FreeLoader ejecutable"
	@echo ""
	@echo "Requisitos:"
	@echo "  - GNU Binutils (as, ld)"
	@echo "  - GCC con soporte para i386"
	@echo "  - Sistema Linux o compatible"

# ============================================================================
# Información del build
# ============================================================================

.PHONY: info
info:
	@echo "=== Información del Sistema de Compilación ==="
	@echo ""
	@echo "Ensamblador: $(AS)"
	@$(AS) --version | head -1
	@echo ""
	@echo "Compilador: $(CC)"
	@$(CC) --version | head -1
	@echo ""
	@echo "Linker: $(LD)"
	@$(LD) --version | head -1
	@echo ""
	@echo "Flags de compilación:"
	@echo "  ASFLAGS  = $(ASFLAGS)"
	@echo "  CFLAGS   = $(CFLAGS)"
	@echo "  LDFLAGS  = $(LDFLAGS)"
