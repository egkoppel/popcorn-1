set(SRCROOT ${SRCROOT}/test)

add_compile_definitions(HUGOS_TEST)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${SRCROOT}/../src ${SRCROOT}/../include)

add_executable(test_runner
        multiboot.cpp
        frame_bump_alloc.cpp ${SRCROOT}/../src/memory/physical_allocators/monotonic_allocator.cpp
        #frame_bitmap_alloc.cpp ${SRCROOT}/../src/memory/physical_allocators/bitmap_allocator.cpp
        range_tree.cpp
        ring_buffer.cpp
        frame_vector.cpp
        vector.cpp
        optional.cpp)
target_link_libraries(test_runner gtest gtest_main)

find_package(fmt REQUIRED)
target_link_libraries(test_runner fmt::fmt)
