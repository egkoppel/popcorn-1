cmake_minimum_required(VERSION 3.14)
project(libconvolution LANGUAGES CXX ASM_NASM VERSION 0.1.0)

add_library(libconvolution STATIC)
target_sources(libconvolution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/crt0.asm)
target_include_directories(libconvolution PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(libconvolution PROPERTIES OUTPUT_NAME convolution)
