# Makefile para Sistema Operativo Personalizado
# Basado en componentes de ReactOS

# Configuración del compilador
CC = gcc
AS = nasm
LD = ld

# Flags de compilación
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
LIB_DIR = lib
BUILD_DIR = build
ISO_DIR = iso

# Archivos de salida
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOT_BIN = $(BUILD_DIR)/boot.bin
ISO_FILE = os.iso

# Archivos fuente del kernel
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_OBJECTS = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/%.o, $(KERNEL_SOURCES))

# Target por defecto
.PHONY: all
all: setup kernel

# Crear directorios necesarios
.PHONY: setup
setup:
	@echo "Creando directorios de compilación..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ISO_DIR)

# Compilar el kernel
.PHONY: kernel
kernel: setup $(KERNEL_BIN)
	@echo "Kernel compilado exitosamente."

$(KERNEL_BIN): $(KERNEL_OBJECTS)
	@echo "Enlazando kernel..."
	$(LD) $(LDFLAGS) -T $(KERNEL_DIR)/linker.ld -o $@ $^

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el bootloader (futuro - requiere FreeLoader de ReactOS)
.PHONY: boot
boot: setup
	@echo "Bootloader no implementado aún."
	@echo "Se integrará FreeLoader de ReactOS en futuras versiones."

# Crear imagen ISO booteable (futuro)
.PHONY: iso
iso: all
	@echo "Generación de ISO no implementada aún."
	@echo "Requiere bootloader funcional y configuración GRUB/FreeLoader."

# Ejecutar en QEMU (para pruebas futuras)
.PHONY: run
run: iso
	@echo "Ejecutando en QEMU..."
	qemu-system-i386 -cdrom $(ISO_FILE)

# Limpiar archivos de compilación
.PHONY: clean
clean:
	@echo "Limpiando archivos de compilación..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(ISO_DIR)
	@rm -f $(ISO_FILE)
	@echo "Limpieza completada."

# Mostrar ayuda
.PHONY: help
help:
	@echo "Makefile para Sistema Operativo Personalizado"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  all     - Compilar todo el proyecto (por defecto)"
	@echo "  kernel  - Compilar solo el kernel"
	@echo "  boot    - Compilar solo el bootloader (futuro)"
	@echo "  iso     - Crear imagen ISO booteable (futuro)"
	@echo "  run     - Ejecutar en QEMU (futuro)"
	@echo "  clean   - Limpiar archivos de compilación"
	@echo "  help    - Mostrar esta ayuda"
