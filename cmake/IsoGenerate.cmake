configure_file(grub.cfg.in grub.cfg)

add_custom_command(
        OUTPUT popcorn.iso
        DEPENDS grub.cfg ramdisk.tar.gz $<TARGET_FILE:convolution>
        COMMAND mkdir -p iso/boot/grub
        COMMAND cp $<TARGET_FILE:convolution> iso/boot/convolution.k
        COMMAND cp ramdisk.tar.gz iso/boot/ramdisk.tar.gz
        COMMAND cp grub.cfg iso/boot/grub/grub.cfg
        COMMAND grub-mkrescue -o popcorn.iso iso
)
add_custom_target(popcorn DEPENDS popcorn.iso)