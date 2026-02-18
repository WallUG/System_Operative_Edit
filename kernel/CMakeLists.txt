add_library(kernel OBJECT
    main.c
    screen.c
    hal/hal.c
    mm/mm.c
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
    ../lib/memory.c
)

target_include_directories(kernel PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/drivers
)
