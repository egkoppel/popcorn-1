cmake_minimum_required(VERSION 3.14)

set(BUILD_ROOT ${CMAKE_BINARY_DIR} CACHE STRING "")
set(LLVM_SOURCES_SOURCES_DIR llvm-sources-sources)
set(USERSPACE_SYSROOT ${BUILD_ROOT}/popcornroot/System)
set(LLVM_SOURCES_BINARY_DIR ${BUILD_ROOT}/toolchain-build/llvm-sources-binary)
set(CLANG_INSTALL ${LLVM_SOURCES_BINARY_DIR}/stage1_install)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

project(Popcorn LANGUAGES NONE VERSION 0.2.0)
include(ExternalProject)
include(CMakeDependentOption)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Ensure build is out of source
if (EXISTS "${PROJECT_BINARY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Out of source build is required")
endif ()

if (${DO_GET_TOOLCHAIN})
    include(GetToolchain)
    return()
endif ()

# =========== Build options ===========
option(ENABLE_KERNEL_UBSAN "Enable UBSan in the kernel" OFF)
cmake_dependent_option(ENABLE_LIBK_UBSAN "Enable UBSan within libk" ON ${ENABLE_KERNEL_UBSAN} OFF)
cmake_dependent_option(ENABLE_LIBKPP_UBSAN "Enable UBSan within libk++" ON ${ENABLE_KERNEL_UBSAN} OFF)
cmake_dependent_option(BUILD_TESTS "Build libk/++ tests" OFF "${CMAKE_SYSTEM_NAME} STREQUAL HOST" OFF)
set(RAMDISK_INCLUDED_DRIVERS "" CACHE STRING "Drivers to include the the ramdisk - usually disk drivers")
option(TOOLCHAIN_FROM_ARTIFACT "Download toolchain if available rather than building LLVM from source" ON)
set(TOOLCHAIN_LLVM_VERSION "15.0.7" CACHE STRING "Version of LLVM to build the toolchain from")
option(TOOLCHAIN_ONLY "Only build toolchain" OFF)
set(TOOLCHAIN_BUILD_PARALLEL "12" CACHE STRING "Number of parallel threads to use for building toolchain")
# =====================================

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/toolchain-build)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sysroot)
list(APPEND ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/toolchain-build)
list(APPEND ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/sysroot)
execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/toolchain-build
        COMMAND ${CMAKE_COMMAND}
        -UCMAKE_TOOLCHAIN_FILE
        -DDO_GET_TOOLCHAIN=ON
        -DTOOLCHAIN_FROM_ARTIFACT=${TOOLCHAIN_FROM_ARTIFACT}
        -DLLVM_VERSION=${TOOLCHAIN_LLVM_VERSION}
        -DSYSROOT=${CMAKE_CURRENT_BINARY_DIR}/sysroot
        -DTOOLCHAIN_BUILD_PARALLEL=${TOOLCHAIN_BUILD_PARALLEL}
        -DBUILD_ROOT=${BUILD_ROOT}
        -G${CMAKE_GENERATOR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE ret
)
if (ret AND NOT ret EQUAL 0)
    message(FATAL_ERROR "Failed to get toolchain")
endif ()

if (NOT TOOLCHAIN_FROM_ARTIFACT)
    message(STATUS "Building toolchain")
    execute_process(
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/toolchain-build
            COMMAND ${CMAKE_COMMAND} --build . --target build_toolchain -j${TOOLCHAIN_BUILD_PARALLEL}
            RESULT_VARIABLE ret
    )
    if (ret AND NOT ret EQUAL 0)
        message(FATAL_ERROR "Failed to build toolchain")
    endif ()
endif ()

if (${TOOLCHAIN_ONLY})
    return()
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


set(PATH_WRAP ${CMAKE_CURRENT_SOURCE_DIR}/cmake/path_wrapper.sh)

# Check for crosscompile
if ((NOT ${CMAKE_CROSSCOMPILING}) OR (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Generic"))
    set(ARCHITECTURE "HOST")
    message(WARNING "Architecture is set to HOST, rather than a native build")
endif ()

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

# =========== Install dependencies ===========
ExternalProject_Add(
        libc
        GIT_REPOSITORY https://github.com/egkoppel/mlibc.git
        GIT_TAG popcorn-latest
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        BUILD_IN_SOURCE true
        INSTALL_DIR ${PROJECT_BINARY_DIR}/popcornroot/System
        CONFIGURE_COMMAND
        ${PATH_WRAP} ${CMAKE_SYSROOT}/bin meson setup --prefix=<INSTALL_DIR> -Dc_args=-fno-stack-protector -Dcpp_args=-fno-stack-protector -Dbuild_tests=false -Ddefault_library=static --cross-file ci/popcorn-clang.cross-file popcorn-build
        BUILD_COMMAND ${PATH_WRAP} ${CMAKE_SYSROOT}/bin ninja -C popcorn-build
        INSTALL_COMMAND ${PATH_WRAP} ${CMAKE_SYSROOT}/bin ninja -C popcorn-build install
)
ExternalProject_Get_property(libc INSTALL_DIR SOURCE_DIR)
list(APPEND ADDITIONAL_CLEAN_FILES ${SOURCE_DIR}/popcorn-build)
list(APPEND ADDITIONAL_CLEAN_FILES ${INSTALL_DIR})

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
# TODO: Fix this - currently a bit of a hack to get around include files
target_compile_options(cxxrt-static PRIVATE "-isystem${INSTALL_DIR}/include")
add_dependencies(cxxrt-static libc)
# ============================================

# Subdirectories
add_subdirectory(docs)
add_subdirectory(libk)
add_subdirectory(libk++)
add_subdirectory(kernel)
add_subdirectory(codesign)

set(SYSROOT_BACKUP ${CMAKE_SYSROOT})
set(CMAKE_SYSROOT ${USERSPACE_SYSROOT})
add_subdirectory(userspace)
set(CMAKE_SYSROOT ${SYSROOT_BACKUP})

include(RamdiskGenerate)
include(IsoGenerate)
