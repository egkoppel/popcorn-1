cmake_minimum_required(VERSION 3.14)
project(libconvolution LANGUAGES CXX VERSION 0.1.0)

add_library(libconvolution INTERFACE)
target_include_directories(libconvolution PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
