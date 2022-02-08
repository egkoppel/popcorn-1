CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
QEMU ?= qemu-system-x86_64
OBJCOPY ?= llvm-objcopy

INCLUDE ?= -Isrc/include

CFLAGS32 ?= -c -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target i386-unknown-none
CFLAGS64 ?= $(INCLUDE) -g -c -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none

BUILD_DIR ?= build
ISODIR ?= $(BUILD_DIR)/isodir

OBJS = $(patsubst src/%,$(BUILD_DIR)/%,$(patsubst %.S,%.S.o,$(wildcard src/bootstrap/*.S)) $(patsubst %.c,%.c.o,$(wildcard src/bootstrap/*.c)) $(patsubst %.c,%.c.o,$(wildcard src/init/*.c)) $(patsubst %.cpp,%.cpp.o,$(wildcard src/main/*.cpp)) $(patsubst %.c,%.c.o,$(wildcard src/main/*.c)))
$(info $(OBJS))
LINKER_SCRIPT ?= src/linker.ld
GRUBCFG = grub.cfg

.PHONY: all default clean run
default: $(BUILD_DIR)/hug.iso
all: $(BUILD_DIR)/hug.iso

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

$(BUILD_DIR)/bootstrap/%.S.o: src/bootstrap/%.S | $(BUILD_DIR)/bootstrap
	$(NASM) -felf64 -g -F dwarf -o $@ $<

$(BUILD_DIR)/bootstrap/%.c.o: src/bootstrap/%.c | $(BUILD_DIR)/bootstrap
	$(CC) $(CFLAGS32) -o $@ $<
	$(OBJCOPY) -O elf64-x86-64 -I elf32-x86-64 $@

$(BUILD_DIR)/init/%.c.o: src/init/%.c | $(BUILD_DIR)/init
	$(CC) $(CFLAGS64) -o $@ $<

$(BUILD_DIR)/main/%.cpp.o: src/main/%.cpp | $(BUILD_DIR)/main
	$(CXX) $(CFLAGS64) -o $@ $<

$(BUILD_DIR)/main/%.c.o: src/main/%.c | $(BUILD_DIR)/main
	$(CC) $(CFLAGS64) -o $@ $<

$(BUILD_DIR)/hug.bin: $(OBJS)
	$(LD) -T $(LINKER_SCRIPT) -o $@ $^

$(BUILD_DIR)/hug.iso: $(BUILD_DIR)/hug.bin $(GRUBCFG) | $(ISODIR)
	cp $(BUILD_DIR)/hug.bin $(ISODIR)/boot/hug.bin
	cp $(GRUBCFG) $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)

clean:
	rm -rf $(BUILD_DIR)
run: $(BUILD_DIR)/hug.iso
	$(QEMU) -cdrom $<
