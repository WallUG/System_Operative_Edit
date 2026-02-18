cmake_minimum_required(VERSION 3.10)
project(SystemOperativeEdit C ASM_NASM)

# Cross-compilation configuration
set(CMAKE_SYSTEM_NAME Generic)

# Compilers
set(CMAKE_C_COMPILER gcc)

# Kernel compilation flags
set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2")

# Subdirectories
add_subdirectory(kernel)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker.ld")

# Boot assembly source and object files
set(BOOT_ASM_SOURCE "${CMAKE_SOURCE_DIR}/boot/boot.asm")
set(BOOT_OBJECT "${CMAKE_BINARY_DIR}/boot.asm.o")

# Compile boot.asm
add_custom_command(
    OUTPUT ${BOOT_OBJECT}
    COMMAND nasm -f elf32 -o ${BOOT_OBJECT} ${BOOT_ASM_SOURCE}
    DEPENDS ${BOOT_ASM_SOURCE}
    COMMENT "Assembling boot.asm"
)

# Compile gdt_flush.asm
set(GDT_FLUSH_SOURCE "${CMAKE_SOURCE_DIR}/kernel/interrupt/gdt_flush.asm")
set(GDT_FLUSH_OBJECT "${CMAKE_BINARY_DIR}/gdt_flush.asm.o")
add_custom_command(
    OUTPUT ${GDT_FLUSH_OBJECT}
    COMMAND nasm -f elf32 -o ${GDT_FLUSH_OBJECT} ${GDT_FLUSH_SOURCE}
    DEPENDS ${GDT_FLUSH_SOURCE}
    COMMENT "Assembling gdt_flush.asm"
)

# Compile idt_load.asm
set(IDT_LOAD_SOURCE "${CMAKE_SOURCE_DIR}/kernel/interrupt/idt_load.asm")
set(IDT_LOAD_OBJECT "${CMAKE_BINARY_DIR}/idt_load.asm.o")
add_custom_command(
    OUTPUT ${IDT_LOAD_OBJECT}
    COMMAND nasm -f elf32 -o ${IDT_LOAD_OBJECT} ${IDT_LOAD_SOURCE}
    DEPENDS ${IDT_LOAD_SOURCE}
    COMMENT "Assembling idt_load.asm"
)

# Kernel target
add_executable(kernel.elf
    ${BOOT_OBJECT}
    ${GDT_FLUSH_OBJECT}
    ${IDT_LOAD_OBJECT}
    $<TARGET_OBJECTS:kernel>
)

set_target_properties(kernel.elf PROPERTIES
    LINK_FLAGS "-m32 -T ${LINKER_SCRIPT} -nostdlib -static -lgcc -Wl,--build-id=none"
    LINK_DEPENDS ${LINKER_SCRIPT}
)
