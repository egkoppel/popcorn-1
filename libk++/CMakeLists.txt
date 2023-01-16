project(libk++
        LANGUAGES CXX
        VERSION 0.1.0)

add_library(k++ STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/new.cpp)
target_include_directories(k++ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
