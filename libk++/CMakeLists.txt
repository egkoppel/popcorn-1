project(libk++
        LANGUAGES CXX
        VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckBuiltin)

message(CHECK_START "Checking builtins")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
unset(missingComponents)

check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/ffsll.cpp "__builtin_ffsll()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/bitcast.cpp "__builtin_bit_cast()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/integer_seq.cpp "__make_integer_seq()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/underlying_type.cpp "__underlying_type()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/constructible.cpp "__is_constructible()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/trivially_constructible.cpp "__is_trivially_constructible()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/nothrow_constructible.cpp "__is_nothrow_constructible()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/trivially_copyable.cpp "__is_trivially_copyable()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_void.cpp "__is_void()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_integral.cpp "__is_integral()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_enum.cpp "__is_enum()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_union.cpp "__is_union()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_class.cpp "__is_class()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_empty.cpp "__is_empty()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_base_of.cpp "__is_base_of()")
check_builtin(${CMAKE_CURRENT_SOURCE_DIR}/cmake/checks/is_convertible_to.cpp "__is_convertible_to()")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if (missingComponents)
    message(CHECK_FAIL "missing builtins: ${missingComponents}")
    message(SEND_ERROR "builtins check failed")
else ()
    message(CHECK_PASS "all required builtins found")
endif ()

add_library(k++ STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/new.cpp)
target_compile_options(k++ PRIVATE ${C_CXX_KERNEL_COMPILE_FLAGS})

if (${ENABLE_LIBKPP_UBSAN})
    target_compile_options(k++ PRIVATE ${UBSAN_FLAG})
endif ()

target_include_directories(k++ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(k++ PUBLIC k)
