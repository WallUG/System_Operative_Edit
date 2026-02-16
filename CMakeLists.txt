cmake_minimum_required(VERSION 3.10)
project(SystemOperativeEdit C ASM_NASM)

# Configuración para cross-compilation
set(CMAKE_SYSTEM_NAME Generic)

# Compiladores
set(CMAKE_C_COMPILER gcc)

# Flags de compilación para kernel
set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2")

# Subdirectorios
add_subdirectory(kernel)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker.ld")

# Boot assembly
set(BOOT_ASM_SOURCE "${CMAKE_SOURCE_DIR}/boot/boot.asm")
set(BOOT_OBJECT "${CMAKE_BINARY_DIR}/boot.asm.o")

# Compile boot.asm manually
add_custom_command(
    OUTPUT ${BOOT_OBJECT}
    COMMAND nasm -f elf32 -o ${BOOT_OBJECT} ${BOOT_ASM_SOURCE}
    DEPENDS ${BOOT_ASM_SOURCE}
    COMMENT "Assembling boot.asm"
)

# Target del kernel
add_executable(kernel.elf
    ${BOOT_OBJECT}
    $<TARGET_OBJECTS:kernel>
)

set_target_properties(kernel.elf PROPERTIES
    LINK_FLAGS "-m32 -T ${LINKER_SCRIPT} -nostdlib -static -lgcc"
    LINK_DEPENDS ${LINKER_SCRIPT}
)
