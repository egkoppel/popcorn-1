cmake_minimum_required(VERSION 3.14)
project(driver_ps2 LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(driver_ps2)
target_sources(driver_ps2 PRIVATE
        main.cpp)
target_link_libraries(driver_ps2 PRIVATE ipc libc)
target_compile_options(driver_ps2 PRIVATE -fno-exceptions -fno-rtti)
