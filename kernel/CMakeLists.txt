cmake_minimum_required(VERSION 3.14)
project(convolution LANGUAGES NONE VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

add_custom_command(
        OUTPUT fonts.o
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts/font.psf
        COMMAND ${CMAKE_OBJCOPY} -O elf64-x86-64 -I binary --prefix-alloc-sections=.font
        --rename-section .data=.rodata ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts/font.psf fonts.o
)

add_executable(convolution
        fonts.o)
target_compile_options(convolution PRIVATE ${C_CXX_KERNEL_COMPILE_FLAGS})
target_compile_options(convolution PRIVATE $<$<COMPILE_LANGUAGE:CXX>:
        -Wall -Werror=return-type -Werror=implicit-fallthrough -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments
        >)

if (${ENABLE_KERNEL_UBSAN})
    target_compile_options(convolution PRIVATE ${UBSAN_FLAG})
endif ()

target_include_directories(convolution PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_options(convolution PRIVATE
        --eh-frame-hdr
        -T ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld)

target_link_libraries(convolution
        k
        k++
        cxxrt-static
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libunwind.a)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
