project("Core kernel")

set(SRCROOT ${PROJECT_SOURCE_DIR})

function(check_builtin source_file function_name)
    message(CHECK_START "Checking for " ${function_name})
    try_compile(CHECK_RESULT ${PROJECT_BINARY_DIR} SOURCES ${source_file} OUTPUT_VARIABLE CHECK_RESULT_OUT)
    if(${CHECK_RESULT})
        message(CHECK_PASS "found")
    else()
        message(CHECK_FAIL "not found")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        message(VERBOSE "Failed with output:\n" ${CHECK_RESULT_OUT})
        list(APPEND missingComponents ${function_name})
        set(missingComponents ${missingComponents} PARENT_SCOPE)
    endif()
endfunction()

message(CHECK_START "Checking builtins")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
unset(missingComponents)

check_builtin(${SRCROOT}/cmake/checks/ffsll.cpp "__builtin_ffsll()")
check_builtin(${SRCROOT}/cmake/checks/bitcast.cpp "__builtin_bit_cast()")
check_builtin(${SRCROOT}/cmake/checks/integer_seq.cpp "__make_integer_seq()")
check_builtin(${SRCROOT}/cmake/checks/underlying_type.cpp "__underlying_type()")
check_builtin(${SRCROOT}/cmake/checks/constructible.cpp "__is_constructible()")
check_builtin(${SRCROOT}/cmake/checks/trivially_constructible.cpp "__is_trivially_constructible()")
check_builtin(${SRCROOT}/cmake/checks/nothrow_constructible.cpp "__is_nothrow_constructible()")
check_builtin(${SRCROOT}/cmake/checks/trivially_copyable.cpp "__is_trivially_copyable()")
check_builtin(${SRCROOT}/cmake/checks/is_void.cpp "__is_void()")
check_builtin(${SRCROOT}/cmake/checks/is_integral.cpp "__is_integral()")
check_builtin(${SRCROOT}/cmake/checks/is_enum.cpp "__is_enum()")
check_builtin(${SRCROOT}/cmake/checks/is_union.cpp "__is_union()")
check_builtin(${SRCROOT}/cmake/checks/is_class.cpp "__is_class()")
check_builtin(${SRCROOT}/cmake/checks/is_empty.cpp "__is_empty()")
check_builtin(${SRCROOT}/cmake/checks/is_base_of.cpp "__is_base_of()")
check_builtin(${SRCROOT}/cmake/checks/is_convertible_to.cpp "__is_convertible_to()")

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(missingComponents)
    message(CHECK_FAIL "missing builtins: ${missingComponents}")
    message(SEND_ERROR "builtins check failed")
else()
    message(CHECK_PASS "all required builtins found")
endif()

add_subdirectory(${SRCROOT}/src)

if(NOT ${CMAKE_CROSSCOMPILING})
    add_subdirectory(${SRCROOT}/test)
endif()