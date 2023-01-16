find_program(TAR NAMES gtar tar)

add_custom_command(OUTPUT initramfs
        COMMAND mkdir initramfs)

add_custom_command(OUTPUT ramdisk.tar.gz
        DEPENDS initramfs
        COMMAND ${TAR} -czf ramdisk.tar.gz initramfs)
