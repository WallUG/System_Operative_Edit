add_library(kernel OBJECT
    main.c
    screen.c
    hal/hal.c
    mm/mm.c
)

target_include_directories(kernel PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
