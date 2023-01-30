add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/arch)

FILE(GLOB_RECURSE non_arch_sources *.c *.cpp)
list(FILTER non_arch_sources EXCLUDE REGEX ".*/arch/.*")

target_sources(convolution PRIVATE ${non_arch_sources})
