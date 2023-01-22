if (${TOOLCHAIN_FROM_ARTIFACT})
    message(FATAL_ERROR "Downloading toolchain")
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
    )
    FetchContent_Populate(llvm-sources)
    FetchContent_GetProperties(llvm-sources
            SOURCE_DIR LLVM_SOURCES_SOURCE_DIR
            BINARY_DIR LLVM_SOURCES_BINARY_DIR)


    message(STATUS "Configuring clang")
    ExternalProject_Add(clang-bootstrap
            BINARY_DIR ${LLVM_SOURCES_BINARY_DIR}/stage1
            SOURCE_DIR ${LLVM_SOURCES_SOURCE_DIR}/llvm
            CMAKE_ARGS
            -DLLVM_ENABLE_PROJECTS=clang\;lld
            -DLLVM_ENABLE_RUNTIMES=
            -DCMAKE_BUILD_TYPE=Release
            -DDEFAULT_SYSROOT=/System
            -DLLVM_TARGETS_TO_BUILD=X86
            -DLLVM_DEFAULT_TARGET_TRIPLE=x86_64-unknown-popcorn
            -DCLANG_ENABLE_BOOTSTRAP=Off
            INSTALL_COMMAND ""
            )

    list(POP_BACK CMAKE_MESSAGE_INDENT)

    add_custom_target(build_toolchain DEPENDS
            clang-bootstrap)
endif ()
