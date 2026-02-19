add_library(kernel OBJECT
    main.c
    boot_splash.c
    screen.c
    hal/hal.c
    mm/mm.c
    mm/pmm.c
    mm/vmm.c
    proc/process.c
    proc/scheduler.c
    interrupt/gdt.c
    interrupt/idt.c
    ../drivers/framework/io_manager.c
    ../drivers/framework/device.c
    ../drivers/hal/display.c
    ../drivers/video/vga/vga_driver.c
    ../drivers/video/vga/vga_init.c
    ../drivers/video/vga/vga_hardware.c
    ../drivers/video/vga/vga_screen.c
    ../drivers/video/vga/vga_operations.c
    ../drivers/video/vga/vga_font.c
    ../drivers/video/vga/vga_mouse.c
    ../drivers/video/vga/vga_gui.c
    ../lib/memory.c
)

target_include_directories(kernel PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/drivers
)
