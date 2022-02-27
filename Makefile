CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
QEMU ?= qemu-system-x86_64
OBJCOPY ?= llvm-objcopy
AR = llvm-ar
CARGO ?= cargo

INCLUDE ?= -Isrc/libk/include

OPT ?= 0
CFLAGS = $(INCLUDE) -O$(OPT) -Wall -Wextra -Wpedantic -Wno-language-extension-token -Werror=incompatible-pointer-types -Wno-address-of-packed-member -mcmodel=large -MMD -MP -c -g -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none-elf -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float
CARGOFLAGS ?=
LDFLAGS ?= 
QEMU_ARGS ?=
QEMU_SERIAL ?= -serial stdio

RUST_BUILD_TYPE ?= undefined
RUST_TARGET_NAME ?= hugos-target

ifeq ($(RUST_BUILD_TYPE),undefined)
ifeq ($(filter $(OPT),2 3),)
RUST_BUILD_TYPE = debug
else
RUST_BUILD_TYPE = release
endif
endif

BUILD_DIR ?= build
ISODIR ?= $(BUILD_DIR)/isodir
RUST_DIR ?= src/rust

RUST_LIB_DIR = $(RUST_DIR)/target/$(RUST_TARGET_NAME)/$(RUST_BUILD_TYPE)

OBJS = $(patsubst src/%,$(BUILD_DIR)/%, \
	$(patsubst %.S,%.S.o,$(wildcard src/bootstrap/*.S)) \
	$(patsubst %.c,%.c.o,$(wildcard src/init/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/main/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/memory/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/interrupts/*.c)) \
	$(patsubst %.psf,%.psf.o,$(wildcard src/fonts/*.psf)))
OBJS_LIBK = $(patsubst src/%,$(BUILD_DIR)/%, \
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
$(BUILD_DIR)/fonts: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/fonts
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

$(BUILD_DIR)/fonts/%.psf.o: src/fonts/%.psf | $(BUILD_DIR)/fonts
	$(OBJCOPY) -O elf64-x86-64 -I binary \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_start=$(subst .,_,$(lastword $(subst /, ,$<)))_start \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_end=$(subst .,_,$(lastword $(subst /, ,$<)))_end \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_size=$(subst .,_,$(lastword $(subst /, ,$<)))_size \
		--rename-section .data=.rodata \
		$< $@

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

$(BUILD_DIR)/libk.a: $(OBJS_LIBK) | $(BUILD_DIR)
	$(AR) -rcs $@ $^

-include $(RUST_LIB_DIR)/libhugos.d
$(RUST_LIB_DIR)/libhugos.a:
ifeq ($(RUST_BUILD_TYPE),release)
	+cd $(RUST_DIR) && $(CARGO) build --release
else
	+cd $(RUST_DIR) && $(CARGO) build
endif

$(BUILD_DIR)/hug.bin: $(OBJS) $(BUILD_DIR)/libk.a $(RUST_LIB_DIR)/libhugos.a
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJS) -L$(BUILD_DIR) -lk -L$(RUST_LIB_DIR) -lhugos

$(BUILD_DIR)/hug.iso: $(BUILD_DIR)/hug.bin $(GRUBCFG) | $(ISODIR)
	cp $(BUILD_DIR)/hug.bin $(ISODIR)/boot/hug.bin
	cp $(GRUBCFG) $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)

clean:
	rm -rf $(BUILD_DIR)
	cd $(RUST_DIR) && $(CARGO) clean
run: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -drive file=$<,format=raw
run_debug: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -s -S -drive file=$<,format=raw
