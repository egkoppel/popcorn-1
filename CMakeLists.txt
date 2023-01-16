cmake_minimum_required(VERSION 3.14)
project(Popcorn LANGUAGES NONE VERSION 0.2.0)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(ExternalProject)
include(CMakeDependentOption)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Ensure build is out of source
if (EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Out of source build is required")
endif ()

# =========== Optimization flags ===========
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})
# ==========================================

# Check for crosscompile
if ((NOT ${CMAKE_CROSSCOMPILING}) OR (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic"))
    set(ARCHITECTURE "HOST")
    message(WARNING "Architecture is set to HOST, rather than a native build")
endif ()

# =========== Build options ===========
option(ENABLE_KERNEL_UBSAN "Enable UBSan in the kernel" OFF)
cmake_dependent_option(ENABLE_LIBK_UBSAN "Enable UBSan within libk" ON ${ENABLE_KERNEL_UBSAN} OFF)
cmake_dependent_option(ENABLE_LIBKPP_UBSAN "Enable UBSan within libk++" ON ${ENABLE_KERNEL_UBSAN} OFF)
cmake_dependent_option(BUILD_TESTS "Build libk/++ tests" OFF "${CMAKE_SYSTEM_NAME} STREQUAL HOST" OFF)
set(RAMDISK_INCLUDED_DRIVERS "" CACHE STRING "Drivers to include the the ramdisk - usually disk drivers")
# =====================================

# =========== Generate extra compiler options ===========
set(C_CXX_KERNEL_COMPILE_FLAGS
        $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:
        -fexceptions
        -mcmodel=kernel
        -nostdlib
        -fno-stack-protector
        -ffreestanding
        -mno-mmx
        -mno-sse
        -mno-sse3
        -mno-sse4
        -mno-avx
        -mno-red-zone
        -msoft-float>)

set(UBSAN_FLAG
        $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:
        "-fsanitize=undefined" >)
# =======================================================

# Subdirectories
add_subdirectory(docs)
add_subdirectory(libk)
add_subdirectory(libk++)
add_subdirectory(codesign)
add_subdirectory(kernel)

# =========== Install dependencies ===========
FetchContent_Declare(
        libcxxrt
        GIT_REPOSITORY https://github.com/libcxxrt/libcxxrt.git
        GIT_TAG a0f7f5c139a7daf71de0de201b6c405d852b1dc1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply --ignore-space-change --ignore-whitespace ${PROJECT_SOURCE_DIR}/cmake/libcxxrt.patch
)
FetchContent_MakeAvailable(libcxxrt)
target_compile_definitions(cxxrt-static PUBLIC LIBCXXRT_WEAK_LOCKS)
target_compile_options(cxxrt-static PRIVATE ${C_CXX_KERNEL_COMPILE_FLAGS})
target_include_directories(cxxrt-static BEFORE PRIVATE $<TARGET_PROPERTY:k,INCLUDE_DIRECTORIES>)
# ============================================

return()

configure_file(grub.cfg.in grub.cfg)

set(WARNINGS "-Wall -Werror=incompatible-pointer-types -Werror=return-type -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant")

if (${ENABLE_UBSAN})
    set(KERNEL_BUILD_FLAGS " ${KERNEL_BUILD_FLAGS} -fsanitize=undefined ")
endif ()

ExternalProject_Add(convolution SOURCE_DIR ${PROJECT_SOURCE_DIR}/kernel
        CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/cross-${ARCHITECTURE}.cmake)
# ExternalProject_Add(libk SOURCE_DIR ${PROJECT_SOURCE_DIR}/libk CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/cross-${ARCHITECTURE}.cmake)

if (${CMAKE_CROSSCOMPILING})
    add_compile_definitions(__ELF__ __x86_64__)
    include_directories(${PROJECT_SOURCE_DIR}/kernel/src/stl ${PROJECT_SOURCE_DIR}/libk/include ${PROJECT_SOURCE_DIR}/kernel/include ${PROJECT_SOURCE_DIR}/kernel/src ${PROJECT_SOURCE_DIR}/kernel/src/arch/${HUGOS_ARCH})
    set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS} ")
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS} ")
    set(CMAKE_ASM_FLAGS " ${CMAKE_ASM_FLAGS} ${WARNINGS} ${KERNEL_BUILD_FLAGS} ")
endif ()

#add_subdirectory(${SRCROOT}/kernel)
#add_subdirectory(${SRCROOT}/libk)
#add_subdirectory(${SRCROOT}/docs)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/initramfs.tar.gz
        DEPENDS ${PROJECT_SOURCE_DIR}/initramfs
        COMMAND ${TAR} -czf ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_SOURCE_DIR}/initramfs
)
add_custom_target(initramfs DEPENDS ${PROJECT_BINARY_DIR}/initramfs.tar.gz)

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/popcorn.iso
        DEPENDS ${PROJECT_BINARY_DIR}/grub.cfg ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_BINARY_DIR}/kernel.bin
        COMMAND mkdir -p ${PROJECT_BINARY_DIR}/iso/boot/grub
        COMMAND cp ${PROJECT_BINARY_DIR}/kernel.bin ${PROJECT_BINARY_DIR}/iso/boot/hug.bin
        COMMAND cp ${PROJECT_BINARY_DIR}/initramfs.tar.gz ${PROJECT_BINARY_DIR}/iso/boot/initramfs.tar.gz
        COMMAND cp ${PROJECT_BINARY_DIR}/grub.cfg ${PROJECT_BINARY_DIR}/iso/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o ${PROJECT_BINARY_DIR}/popcorn.iso ${PROJECT_BINARY_DIR}/iso
)
add_custom_target(popcorn_iso DEPENDS ${PROJECT_BINARY_DIR}/popcorn.iso)
