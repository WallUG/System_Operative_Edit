# Makefile para Sistema Operativo Personalizado
# Basado en componentes de ReactOS

# Configuración del compilador
CC = gcc
AS = nasm
LD = ld

# Flags de compilación
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -I./include -I./drivers
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
KERNEL_SOURCES = $(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(KERNEL_DIR)/**/*.c)
KERNEL_OBJECTS = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/kernel_%.o, $(KERNEL_SOURCES))

# Archivos ASM del kernel (gdt_flush.asm, idt_load.asm)
KERNEL_ASM_SOURCES = $(wildcard $(KERNEL_DIR)/interrupt/*.asm)
KERNEL_ASM_OBJECTS = $(patsubst $(KERNEL_DIR)/%.asm, $(BUILD_DIR)/kernel_%.o, $(KERNEL_ASM_SOURCES))

# Archivos fuente de drivers
DRIVER_FRAMEWORK_SOURCES = $(wildcard $(DRIVERS_DIR)/framework/*.c)
DRIVER_VIDEO_SOURCES = $(wildcard $(DRIVERS_DIR)/video/vga/*.c)
DRIVER_HAL_SOURCES = $(wildcard $(DRIVERS_DIR)/hal/*.c)
DRIVER_SOURCES = $(DRIVER_FRAMEWORK_SOURCES) $(DRIVER_VIDEO_SOURCES) $(DRIVER_HAL_SOURCES)
DRIVER_OBJECTS = $(patsubst $(DRIVERS_DIR)/%.c, $(BUILD_DIR)/drivers_%.o, $(DRIVER_SOURCES))

# Archivos fuente de lib
LIB_SOURCES = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJECTS = $(patsubst $(LIB_DIR)/%.c, $(BUILD_DIR)/lib_%.o, $(LIB_SOURCES))

# Todos los objetos (boot + kernel + kernel_asm + drivers + lib)
ALL_OBJECTS = $(BOOT_OBJ) $(KERNEL_OBJECTS) $(KERNEL_ASM_OBJECTS) $(DRIVER_OBJECTS) $(LIB_OBJECTS)

# Target por defecto
.PHONY: all
all: setup kernel

# Crear directorios necesarios
.PHONY: setup
setup:
	@echo "Creando directorios de compilación..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ISO_DIR)

# Compilar boot.asm
$(BOOT_OBJ): $(BOOT_DIR)/boot.asm
	@echo "Ensamblando boot.asm..."
	$(AS) $(ASFLAGS) $< -o $@

# Compilar el kernel
.PHONY: kernel
kernel: setup $(BOOT_OBJ) $(KERNEL_BIN)
	@echo "Kernel compilado exitosamente."

$(KERNEL_BIN): $(ALL_OBJECTS)
	@echo "Enlazando kernel..."
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $^

$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c
	@echo "Compilando $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_interrupt/%.o: $(KERNEL_DIR)/interrupt/%.asm
	@echo "Ensamblando $<..."
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/drivers_%.o: $(DRIVERS_DIR)/%.c
	@echo "Compilando driver $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib_%.o: $(LIB_DIR)/%.c
	@echo "Compilando lib $<..."
	@mkdir -p $(dir $@)
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