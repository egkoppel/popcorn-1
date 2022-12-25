project("libk")

set(SRCROOT ${PROJECT_SOURCE_DIR})

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_RANLIB ${CROSS_RANLIB})
set(CMAKE_AR ${CROSS_AR})
include_directories(${SRCROOT}/include ${SRCROOT}/../kernel/include)

add_library(hugos_libk STATIC)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")

target_sources(hugos_libk PRIVATE
        src/malloc.c
        src/stdio.c
        src/stdlib.c
        src/string.c
        src/errno.c)