set(SRCROOT ${SRCROOT}/arch)

if(DEFINED HUGOS_ARCH)
    add_subdirectory(${SRCROOT}/${HUGOS_ARCH})
else()
    message(FATAL_ERROR "No arch defined")
endif()
