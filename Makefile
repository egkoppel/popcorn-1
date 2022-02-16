CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
QEMU ?= qemu-system-x86_64
LDFLAGS ?= 

INCLUDE ?= -Isrc/libk/include

CFLAGS = $(INCLUDE) -Wall -Wextra -Wpedantic -Wno-language-extension-token -Werror=incompatible-pointer-types -mcmodel=large -MMD -MP -c -g -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none-elf -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float
QEMU_ARGS ?=-serial stdio

BUILD_DIR ?= build
ISODIR ?= $(BUILD_DIR)/isodir

OBJS = $(patsubst src/%,$(BUILD_DIR)/%, \
	$(patsubst %.S,%.S.o,$(wildcard src/bootstrap/*.S)) \
	$(patsubst %.c,%.c.o,$(wildcard src/init/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/main/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/memory/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/interrupts/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/libk/src/*.c)))
LINKER_SCRIPT ?= src/linker.ld
GRUBCFG = src/grub.cfg

DEPENDS = $(patsubst %.o, %.d, $(OBJS))

.PHONY: all default clean run run_debug
default: $(BUILD_DIR)/hug.iso
all: $(BUILD_DIR)/hug.iso

-include $(DEPENDS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(ISODIR): | $(BUILD_DIR)
	mkdir -p $(ISODIR)/boot/grub
$(BUILD_DIR)/bootstrap: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/bootstrap
$(BUILD_DIR)/init: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/init
$(BUILD_DIR)/main: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/main
$(BUILD_DIR)/memory: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/memory
$(BUILD_DIR)/libk/src: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libk/src
$(BUILD_DIR)/interrupts: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/interrupts

$(BUILD_DIR)/bootstrap/%.S.o: src/bootstrap/%.S | $(BUILD_DIR)/bootstrap
	$(NASM) -felf64 -g -F dwarf -o $@ $<

$(BUILD_DIR)/init/%.c.o: src/init/%.c | $(BUILD_DIR)/init
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/main/%.c.o: src/main/%.c | $(BUILD_DIR)/main
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/memory/%.c.o: src/memory/%.c | $(BUILD_DIR)/memory
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/interrupts/%.c.o: src/interrupts/%.c | $(BUILD_DIR)/interrupts
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/libk/src/%.c.o: src/libk/src/%.c | $(BUILD_DIR)/libk/src
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/hug.bin: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $^

$(BUILD_DIR)/hug.iso: $(BUILD_DIR)/hug.bin $(GRUBCFG) | $(ISODIR)
	cp $(BUILD_DIR)/hug.bin $(ISODIR)/boot/hug.bin
	cp $(GRUBCFG) $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)

clean:
	rm -rf $(BUILD_DIR)
run: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) -drive file=$<,format=raw
run_debug: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) -s -S -drive file=$<,format=raw
