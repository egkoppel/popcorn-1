cmake_minimum_required(VERSION 3.14)
project(libipc LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_library(ipc STATIC)
target_include_directories(ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ipc PRIVATE
        src/client.cpp
        src/ipc.cpp
        src/serialization.cpp
        src/server.cpp)
target_compile_options(ipc PRIVATE -fno-exceptions -fno-rtti)
target_link_libraries(ipc PUBLIC libc)
