cmake_minimum_required(VERSION 3.14)
project(popcorn LANGUAGES NONE)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if (EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Out of source build is required")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

find_program(TAR tar)

set(triple x86_64-unknown-none-elf)

set(WARNINGS "-Wall -Werror=incompatible-pointer-types -Werror=return-type -Werror=implicit-fallthrough -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant")
set(KERNEL_BUILD_FLAGS "-ferror-limit=0 -fexceptions -mcmodel=kernel -nostdlib -fno-stack-protector -ffreestanding -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float")

set(SRCROOT ${PROJECT_SOURCE_DIR})

add_subdirectory(${SRCROOT}/kernel)
add_subdirectory(${SRCROOT}/libk)
add_subdirectory(${SRCROOT}/docs)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/kernel.bin
        OUTPUT ${PROJECT_BINARY_DIR}/convolution.map
        DEPENDS hugos_core_kernel hugos_libk ${SRCROOT}/kernel/src/linker.ld /Users/Eliyahu/Downloads/libunwind-15.0.5.src/build/lib/libunwind.a /Users/Eliyahu/libcxxrt/build/lib/libcxxrt.a
        COMMAND ${CMAKE_CXX_LINK_EXECUTABLE} --eh-frame-hdr --error-limit=0 -T ${SRCROOT}/kernel/src/linker.ld -o ${PROJECT_BINARY_DIR}/kernel.bin $<TARGET_FILE:hugos_core_kernel> $<TARGET_FILE:hugos_libk> ${PROJECT_BINARY_DIR}/kernel/src/CMakeFiles/hugos_core_kernel.dir/arch/amd64/bootstrap/header.asm.obj ${PROJECT_BINARY_DIR}/kernel/fonts.o
        /Users/Eliyahu/Downloads/libunwind-15.0.5.src/build/lib/libunwind.a /Users/Eliyahu/libcxxrt/build/lib/libcxxrt.a
        COMMAND ${CMAKE_NM} -C ${PROJECT_BINARY_DIR}/kernel.bin > ${PROJECT_BINARY_DIR}/convolution.map
)
add_custom_target(hugos_kernel_bin DEPENDS ${PROJECT_BINARY_DIR}/kernel.bin)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/initramfs.tar.gz
        DEPENDS ${SRCROOT}/initramfs
        COMMAND ${TAR} -czf ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${SRCROOT}/initramfs
)
add_custom_target(hugos_initramfs DEPENDS ${PROJECT_BINARY_DIR}/initramfs.tar.gz)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/popcorn.iso
        DEPENDS ${SRCROOT}/grub.cfg hugos_initramfs ${PROJECT_BINARY_DIR}/kernel.bin
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/iso/boot/grub
        COMMAND cp ${PROJECT_BINARY_DIR}/kernel.bin ${PROJECT_BINARY_DIR}/iso/boot/hug.bin
        COMMAND cp ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_BINARY_DIR}/iso/boot/initramfs.tar.gz
        COMMAND cp ${SRCROOT}/grub.cfg ${PROJECT_BINARY_DIR}/iso/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${PROJECT_BINARY_DIR}/popcorn.iso ${PROJECT_BINARY_DIR}/iso
)
add_custom_target(hugos_iso DEPENDS ${PROJECT_BINARY_DIR}/popcorn.iso)