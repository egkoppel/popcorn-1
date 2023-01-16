if (${CMAKE_CROSSCOMPILING})
    ExternalProject_Add(codesign
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
            CMAKE_ARGS -U CMAKE_TOOLCHAIN_FILE
            INSTALL_COMMAND ""
            )
    return()
endif ()

cmake_minimum_required(VERSION 3.14)
project(PopcornCodesign
        LANGUAGES CXX
        VERSION 0.0.0)

add_executable(codesign)
target_sources(codesign PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
