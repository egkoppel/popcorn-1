find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

set(DOXYGEN_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/html/index.html)
file(GLOB_RECURSE POPCORN_HEADERS ${SRCROOT}/kernel/**/*.hpp ${SRCROOT}/kernel/**/*.h ${SRCROOT}/libk/**/*.h)

set(DOXYGEN_INPUT_DIR "${PROJECT_SOURCE_DIR}/kernel ${PROJECT_SOURCE_DIR}/libk")
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
        DEPENDS ${POPCORN_HEADERS}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating docs"
        VERBATIM)

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)

add_custom_target(Sphinx ALL
        COMMAND ${SPHINX_EXECUTABLE} -b html
        # Tell Breathe where to find the Doxygen output
        -Dbreathe_projects.popcorn=${DOXYGEN_OUTPUT_DIR}/xml
        ${SPHINX_SOURCE} ${SPHINX_BUILD}
        DEPENDS Doxygen
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Sphinx")