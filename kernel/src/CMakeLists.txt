add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/arch)

FILE(GLOB_RECURSE non_arch_sources *.c *.cpp)
list(FILTER non_arch_sources EXCLUDE REGEX ".*/arch/.*")

target_sources(convolution PRIVATE ${non_arch_sources})

return()

target_sources(core PRIVATE "${CMAKE_SOURCE_DIR}/idl/include/ipc/popcorn/input/keyboard_client.cpp"
        "${CMAKE_SOURCE_DIR}/idl/include/ipc/popcorn/input/keyboard_server.cpp"
        "${CMAKE_SOURCE_DIR}/idl/libipc/src/server.cpp"
        "${CMAKE_SOURCE_DIR}/idl/libipc/src/client.cpp"
        "${CMAKE_SOURCE_DIR}/idl/libipc/src/ipc.cpp" "${CMAKE_SOURCE_DIR}/idl/libipc/src/serialization.cpp")
target_include_directories(core PRIVATE "${CMAKE_SOURCE_DIR}/idl/include" "${CMAKE_SOURCE_DIR}/idl/libipc/include")
