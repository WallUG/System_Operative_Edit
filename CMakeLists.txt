cmake_minimum_required(VERSION 3.10)
project(SystemOperativeEdit C ASM_NASM)

# Enable NASM
enable_language(ASM_NASM)

# Configuración para cross-compilation
set(CMAKE_SYSTEM_NAME Generic)

# Compiladores
set(CMAKE_C_COMPILER gcc)
set(CMAKE_ASM_NASM_COMPILER nasm)

# Flags de compilación para kernel
set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2")
set(CMAKE_ASM_NASM_FLAGS "-f elf32")

# Subdirectorios
add_subdirectory(boot)
add_subdirectory(kernel)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker.ld")

# Target del kernel
add_executable(kernel.elf
    $<TARGET_OBJECTS:boot>
    $<TARGET_OBJECTS:kernel>
)

set_target_properties(kernel.elf PROPERTIES
    LINK_FLAGS "-m32 -T ${LINKER_SCRIPT} -nostdlib -lgcc"
    LINK_DEPENDS ${LINKER_SCRIPT}
)
