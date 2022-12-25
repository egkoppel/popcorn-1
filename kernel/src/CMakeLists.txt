set(SRCROOT ${SRCROOT}/src)

set(CMAKE_RANLIB ${CROSS_RANLIB})
set(CMAKE_AR ${CROSS_AR})

add_compile_definitions(FMT_HEADER_ONLY)

include_directories(${SRCROOT}/stl ${SRCROOT}/../../libk/include ${SRCROOT}/../include ${SRCROOT})
include_directories(${SRCROOT}/../extern/fmt/include)

add_library(hugos_core_kernel STATIC)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")

target_sources(hugos_core_kernel PRIVATE "${SRCROOT}/../cmake/generators/task_hpp_gen_nasm_includes.cpp")

        add_subdirectory(${SRCROOT}/arch)

FILE(GLOB_RECURSE non_arch_sources *.c *.cpp)
list(FILTER non_arch_sources EXCLUDE REGEX ".*/arch/.*")

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/fonts.o
        DEPENDS ${SRCROOT}/fonts/font.psf
        COMMAND ${CMAKE_OBJCOPY} -O elf64-x86-64 -I binary --prefix-alloc-sections=.font
        --rename-section .data=.rodata ${SRCROOT}/fonts/font.psf ${PROJECT_BINARY_DIR}/fonts.o
)

target_sources(hugos_core_kernel PRIVATE ${non_arch_sources} ${PROJECT_BINARY_DIR}/fonts.o)
