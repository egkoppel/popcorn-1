cmake_minimum_required(VERSION 3.14)
project(test_server LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(server_test)
target_sources(server_test PRIVATE
        main.cpp)
add_dependencies(server_test libc)
target_compile_options(server_test PRIVATE -fno-exceptions -fno-rtti)

