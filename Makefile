QEMU ?= qemu-system-x86_64
TAR ?= tar

QEMU_ARGS ?= -smp 2
QEMU_SERIAL ?= -serial stdio

BUILD_DIR ?= build
ISODIR ?= $(BUILD_DIR)/isodir

LINKER_SCRIPT ?= src/linker.ld
LINKER_SCRIPT_RAMFS ?= src/linker_userspace.ld
GRUBCFG = src/grub.cfg

RAMDISK_IMG = $(BUILD_DIR)/initramfs.tar.gz
RAMDISK = initramfs

.PHONY: all default clean run run_debug src/libk/$(BUILD_DIR)/libk.a src/kernel/$(BUILD_DIR)/hug.bin
default: $(BUILD_DIR)/hug.iso
all: default

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ISODIR): | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/isodir/boot/grub

$(RAMDISK_IMG): $(RAMDISK) | $(BUILD_DIR)
	$(TAR) -czf $@ $<

src/libk/$(BUILD_DIR)/libk.a:
	$(MAKE) -C src/libk

src/kernel/$(BUILD_DIR)/hug.bin: src/libk/$(BUILD_DIR)/libk.a
	$(MAKE) -C src/kernel

$(BUILD_DIR)/hug.iso: src/kernel/$(BUILD_DIR)/hug.bin $(GRUBCFG) $(RAMDISK_IMG) | $(ISODIR)
	cp src/kernel/$(BUILD_DIR)/hug.bin $(ISODIR)/boot/hug.bin
	cp $(RAMDISK_IMG) $(ISODIR)/boot/initramfs.tar.gz
	cp $(GRUBCFG) $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) clean -C src/kernel
	$(MAKE) clean -C src/libk
run: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -drive file=$<,format=raw
run_debug: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -s -S -drive file=$<,format=raw
