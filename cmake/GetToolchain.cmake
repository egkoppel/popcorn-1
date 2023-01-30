if (${TOOLCHAIN_FROM_ARTIFACT})
    message(STATUS "Downloading toolchain")

    if (APPLE)
        set(TOOLCHAIN_OS darwin)
    else ()
        message(FATAL_ERROR No toolchain available for ${CMAKE_SYSTEM_NAME})
    endif ()

    FetchContent_Declare(
            toolchain
            SOURCE_DIR ${CLANG_INSTALL}
            URL https://github.com/egkoppel/popcorn/raw/toolchain/toolchain-${TOOLCHAIN_OS}.tar.gz
            URL_HASH SHA256=8597c21af31f17f9e6a13051554709ccfb5229738d3e6dbfb4df812d12d12f3a
    )
    FetchContent_Populate(toolchain)
else ()
    message(STATUS "Building LLVM version ${LLVM_VERSION}")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")

    # Get LLVM sources
    message(STATUS "Downloading LLVM: https://github.com/llvm/llvm-project.git:llvmorg-${LLVM_VERSION}")
    FetchContent_Declare(
            llvm-sources
            GIT_REPOSITORY https://github.com/llvm/llvm-project.git
            GIT_TAG llvmorg-${LLVM_VERSION}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            SOURCE_DIR ${LLVM_SOURCES_SOURCES_DIR}
            BINARY_DIR ${LLVM_SOURCES_BINARY_DIR}
    )
    FetchContent_Populate(llvm-sources)

    message(STATUS "Configuring clang")
    ExternalProject_Add(clang-bootstrap
            BINARY_DIR ${LLVM_SOURCES_BINARY_DIR}/stage1
            SOURCE_DIR ${LLVM_SOURCES_SOURCE_DIR}/llvm
            CMAKE_ARGS
            -DLLVM_ENABLE_PROJECTS=clang
            -DLLVM_ENABLE_RUNTIMES=
            -DCMAKE_BUILD_TYPE=Release
            -DDEFAULT_SYSROOT=/System
            -DLLVM_TARGETS_TO_BUILD=X86
            -DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-popcorn
            -DCLANG_ENABLE_BOOTSTRAP=Off
            -DLLVM_BUILD_TESTS=OFF
            -DCMAKE_INSTALL_PREFIX=${CLANG_INSTALL}
            BUILD_COMMAND ${CMAKE_COMMAND} --build . --target clang -j${TOOLCHAIN_BUILD_PARALLEL}
            INSTALL_COMMAND ""
            )

    list(POP_BACK CMAKE_MESSAGE_INDENT)

    add_custom_target(build_toolchain DEPENDS
            clang-bootstrap)
endif ()
