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
KERNEL_BIN = $(BUILD_DIR)/kernel.elf
BOOT_OBJ = $(BUILD_DIR)/boot.o
ISO_FILE = os.iso

# Archivos fuente del kernel
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_OBJECTS = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/%.o, $(KERNEL_SOURCES))

# Todos los objetos (boot + kernel)
ALL_OBJECTS = $(BOOT_OBJ) $(KERNEL_OBJECTS)

# Target por defecto
.PHONY: all
all: setup kernel

# Crear directorios necesarios
.PHONY: setup
setup:
\t@echo "Creando directorios de compilación..."
\t@mkdir -p $(BUILD_DIR)
\t@mkdir -p $(ISO_DIR)

# Compilar boot.asm
$(BOOT_OBJ): $(BOOT_DIR)/boot.asm
\t@echo "Ensamblando boot.asm..."
\t$(AS) $(ASFLAGS) $< -o $@

# Compilar el kernel
.PHONY: kernel
kernel: setup $(BOOT_OBJ) $(KERNEL_BIN)
\t@echo "Kernel compilado exitosamente."

$(KERNEL_BIN): $(ALL_OBJECTS)
\t@echo "Enlazando kernel..."
\t$(LD) $(LDFLAGS) -T linker.ld -o $@ $^

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
\t@echo "Compilando $<..."
\t$(CC) $(CFLAGS) -c $< -o $@

# Compilar el bootloader (futuro - requiere FreeLoader de ReactOS)
.PHONY: boot
boot: setup
\t@echo "Bootloader no implementado aún."
\t@echo "Se integrará FreeLoader de ReactOS en futuras versiones."

# Crear imagen ISO booteable (futuro)
.PHONY: iso
iso: all
\t@echo "Generación de ISO no implementada aún."
\t@echo "Requiere bootloader funcional y configuración GRUB/FreeLoader."

# Ejecutar en QEMU (para pruebas futuras)
.PHONY: run
run: iso
\t@echo "Ejecutando en QEMU..."
\tqemu-system-i386 -cdrom $(ISO_FILE)

# Limpiar archivos de compilación
.PHONY: clean
clean:
\t@echo "Limpiando archivos de compilación..."
\t@rm -rf $(BUILD_DIR)
\t@rm -rf $(ISO_DIR)
\t@rm -f $(ISO_FILE)
\t@echo "Limpieza completada."

# Mostrar ayuda
.PHONY: help
help:
\t@echo "Makefile para Sistema Operativo Personalizado"
\t@echo ""
\t@echo "Targets disponibles:"
\t@echo "  all     - Compilar todo el proyecto (por defecto)"
\t@echo "  kernel  - Compilar solo el kernel"
\t@echo "  boot    - Compilar solo el bootloader (futuro)"
\t@echo "  iso     - Crear imagen ISO booteable (futuro)"
\t@echo "  run     - Ejecutar en QEMU (futuro)"
\t@echo "  clean   - Limpiar archivos de compilación"
\t@echo "  help    - Mostrar esta ayuda"