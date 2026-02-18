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

# Compile boot.asm manually because CMake's ASM_NASM support doesn't handle elf32 format correctly
add_custom_command(
    OUTPUT ${BOOT_OBJECT}
    COMMAND nasm -f elf32 -o ${BOOT_OBJECT} ${BOOT_ASM_SOURCE}
    DEPENDS ${BOOT_ASM_SOURCE}
    COMMENT "Assembling boot.asm"
)

# Kernel target
add_executable(kernel.elf
    ${BOOT_OBJECT}
    $<TARGET_OBJECTS:kernel>
)

set_target_properties(kernel.elf PROPERTIES
    LINK_FLAGS "-m32 -T ${LINKER_SCRIPT} -nostdlib -static -lgcc -Wl,--build-id=none"
    LINK_DEPENDS ${LINKER_SCRIPT}
)
