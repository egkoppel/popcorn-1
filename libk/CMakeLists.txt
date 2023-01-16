project(libk
        LANGUAGES C
        VERSION 1.0.0)

add_library(k STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/malloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stdio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/errno.c)
target_include_directories(k PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(k PRIVATE ${CMAKE_SOURCE_DIR}/kernel/include)
