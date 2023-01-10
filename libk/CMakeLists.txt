project("libk")

set(SRCROOT ${PROJECT_SOURCE_DIR})

include_directories(${SRCROOT}/include ${SRCROOT}/../kernel/include)

add_library(hugos_libk STATIC)

target_sources(hugos_libk PRIVATE
        src/malloc.c
        src/stdio.c
        src/stdlib.c
        src/string.c
        src/errno.c)
        