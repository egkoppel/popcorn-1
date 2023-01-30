find_program(TAR NAMES gtar tar)

set(RAMDISK_GENERATE_COMMAND mkdir initramfs)

foreach (DRIVER ${RAMDISK_INCLUDED_DRIVERS})
    set(RAMDISK_GENERATE_COMMAND ${RAMDISK_GENERATE_COMMAND} && cp $<TARGET_FILE:${DRIVER}> initramfs)
endforeach ()

add_custom_command(OUTPUT initramfs
        DEPENDS ${RAMDISK_INCLUDED_DRIVERS}
        COMMAND ${RAMDISK_GENERATE_COMMAND})

add_custom_command(OUTPUT ramdisk.tar.gz
        DEPENDS initramfs ${RAMDISK_INCLUDED_DRIVERS}
        COMMAND ${TAR} -czf ramdisk.tar.gz initramfs)

list(APPEND ADDITIONAL_CLEAN_FILES "initramfs" "ramdisk.tar.gz")
