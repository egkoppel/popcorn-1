# Popcorn doesn't support shared libraries
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

# To help the find_xxx() commands, set at least the following so CMAKE_FIND_ROOT_PATH
# works at least for some simple cases:
set(CMAKE_SYSTEM_INCLUDE_PATH /include )
set(CMAKE_SYSTEM_LIBRARY_PATH /lib )
set(CMAKE_SYSTEM_PROGRAM_PATH /bin )

set(CMAKE_EXECUTABLE_SUFFIX .exec)

