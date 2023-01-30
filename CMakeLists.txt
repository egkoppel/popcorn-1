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
        -G${CMAKE_GENERATOR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE ret
)
if (ret AND NOT ret EQUAL 0)
    message(FATAL_ERROR "Failed to get toolchain")
endif ()
execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/toolchain-build
        COMMAND ${CMAKE_COMMAND} --build . --target build_toolchain
        RESULT_VARIABLE ret
)
if (ret AND NOT ret EQUAL 0)
    message(FATAL_ERROR "Failed to get toolchain")
endif ()

return()

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

# Subdirectories
add_subdirectory(docs)
add_subdirectory(libk)
add_subdirectory(libk++)
add_subdirectory(codesign)
add_subdirectory(kernel)

include(RamdiskGenerate)
include(IsoGenerate)
