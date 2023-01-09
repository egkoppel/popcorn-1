cmake_minimum_required(VERSION 3.14)
project(popcorn LANGUAGES NONE)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if (EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Out of source build is required")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

find_program(TAR tar)

set(triple x86_64-unknown-none-elf)

set(WARNINGS "-Wall -Werror=incompatible-pointer-types -Werror=return-type -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant")
set(KERNEL_BUILD_FLAGS "-ferror-limit=0 -fexceptions -mcmodel=kernel -nostdlib -fno-stack-protector -ffreestanding -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

set(SRCROOT ${PROJECT_SOURCE_DIR})

include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        libcxxrt
        GIT_REPOSITORY https://github.com/libcxxrt/libcxxrt.git
        GIT_TAG a0f7f5c139a7daf71de0de201b6c405d852b1dc1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply --ignore-space-change --ignore-whitespace ${SRCROOT}/cmake/libcxxrt.patch
)

add_compile_definitions(LIBCXXRT_WEAK_LOCKS)

if (${CMAKE_CROSSCOMPILING})
    add_compile_definitions(__ELF__ __x86_64__)
    include_directories(${SRCROOT}/kernel/src/stl ${SRCROOT}/libk/include ${SRCROOT}/kernel/include ${SRCROOT}/kernel/src ${SRCROOT}/kernel/src/arch/${HUGOS_ARCH})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS}")
endif ()

add_subdirectory(${SRCROOT}/kernel)
add_subdirectory(${SRCROOT}/libk)
add_subdirectory(${SRCROOT}/docs)

FetchContent_MakeAvailable(libcxxrt)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/kernel.bin
        OUTPUT ${PROJECT_BINARY_DIR}/convolution.map
        DEPENDS hugos_core_kernel hugos_libk cxxrt-static ${SRCROOT}/kernel/src/linker.ld /Users/Eliyahu/Downloads/libunwind-15.0.5.src/build/lib/libunwind.a
        COMMAND ${CMAKE_CXX_LINK_EXECUTABLE} --eh-frame-hdr --error-limit=0 -T ${SRCROOT}/kernel/src/linker.ld -o ${PROJECT_BINARY_DIR}/kernel.bin --whole-archive $<TARGET_FILE:hugos_core_kernel> --no-whole-archive $<TARGET_FILE:hugos_libk> $<TARGET_FILE:cxxrt-static>
        /Users/Eliyahu/Downloads/libunwind-15.0.5.src/build/lib/libunwind.a
        COMMAND ${CMAKE_NM} -C ${PROJECT_BINARY_DIR}/kernel.bin > ${PROJECT_BINARY_DIR}/convolution.map
)
add_custom_target(convolution DEPENDS ${PROJECT_BINARY_DIR}/kernel.bin)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/initramfs.tar.gz
        DEPENDS ${SRCROOT}/initramfs
        COMMAND ${TAR} -czf ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${SRCROOT}/initramfs
)
add_custom_target(initramfs DEPENDS ${PROJECT_BINARY_DIR}/initramfs.tar.gz)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/popcorn.iso
        DEPENDS ${SRCROOT}/grub.cfg ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_BINARY_DIR}/kernel.bin
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/iso/boot/grub
        COMMAND cp ${PROJECT_BINARY_DIR}/kernel.bin ${PROJECT_BINARY_DIR}/iso/boot/hug.bin
        COMMAND cp ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_BINARY_DIR}/iso/boot/initramfs.tar.gz
        COMMAND cp ${SRCROOT}/grub.cfg ${PROJECT_BINARY_DIR}/iso/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${PROJECT_BINARY_DIR}/popcorn.iso ${PROJECT_BINARY_DIR}/iso
)
add_custom_target(popcorn_iso DEPENDS ${PROJECT_BINARY_DIR}/popcorn.iso)
