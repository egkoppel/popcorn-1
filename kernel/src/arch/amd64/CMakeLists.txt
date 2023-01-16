FILE(GLOB_RECURSE arch_asm_sources *.asm)
FILE(GLOB_RECURSE arch_cxx_sources *.cpp)

target_sources(convolution PRIVATE ${arch_asm_sources} ${arch_cxx_sources})
target_include_directories(convolution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
