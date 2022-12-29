set(SRCROOT ${SRCROOT}/src)

set(CMAKE_RANLIB ${CROSS_RANLIB})
set(CMAKE_AR ${CROSS_AR})

add_library(hugos_core_kernel STATIC)

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
