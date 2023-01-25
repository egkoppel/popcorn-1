cmake_minimum_required(VERSION 3.14)
project(server_input LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_executable(server_input)
target_sources(server_input PRIVATE
        src/main.cpp
        src/keyboard_client.cpp
        src/keyboard_server.cpp)
target_link_libraries(server_input PRIVATE ipc libc)
target_compile_options(server_input PRIVATE -fno-exceptions -fno-rtti)
