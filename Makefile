CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
QEMU ?= qemu-system-x86_64
OBJCOPY ?= llvm-objcopy
AR = llvm-ar
CARGO ?= cargo
TAR ?= tar

INCLUDE ?= -Isrc/libk/include -Isrc/stlport -Isrc/memory
INCLUDE_TEST ?= -Isrc/libk/include -Isrc/memory

OPT ?= 0
WARNINGS = -Wall -Wextra -Wpedantic -Wno-language-extension-token -Werror=incompatible-pointer-types -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant
CFLAGS_TEST = $(INCLUDE_TEST) -O$(OPT) $(WARNINGS) -MMD -MP -c -g -Wno-incompatible-library-redeclaration
CFLAGS = $(INCLUDE) -O$(OPT) $(WARNINGS) -mcmodel=large -MMD -MP -c -g -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none-elf -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float
CXXFLAGS = -std=c++20
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
RUST_DIR ?= $(shell pwd)/src/rust

RUST_LIB_DIR = $(RUST_DIR)/target/$(RUST_TARGET_NAME)/$(RUST_BUILD_TYPE)

OBJS = $(patsubst src/%,$(BUILD_DIR)/%, \
	$(patsubst %.S,%.S.o,$(wildcard src/bootstrap/*.S)) \
	$(patsubst %.c,%.c.o,$(wildcard src/init/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard src/main/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/main/*.cpp)) \
	$(patsubst %.c,%.c.o,$(wildcard src/memory/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/memory/*.cpp)) \
	$(patsubst %.c,%.c.o,$(wildcard src/interrupts/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/interrupts/*.cpp)) \
	$(patsubst %.c,%.c.o,$(wildcard src/gdt/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/gdt/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/threading/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/*.cpp)) \
	$(patsubst %.psf,%.psf.o,$(wildcard src/fonts/*.psf)))
OBJS_LIBK = $(patsubst src/%,$(BUILD_DIR)/%, \
	$(patsubst %.c,%.c.o,$(wildcard src/libk/src/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/libk/src/*.cpp)))
OBJS_LIBK_TEST = $(patsubst src/%,$(BUILD_DIR)/%, \
	$(patsubst %.c,%.c.test.o, src/libk/src/malloc.c) \
	$(patsubst %.c,%.c.o,$(wildcard src/libk/test/*.c)))
LINKER_SCRIPT ?= src/linker.ld
GRUBCFG = src/grub.cfg

RAMDISK_IMG = $(BUILD_DIR)/initramfs.tar.gz
RAMDISK = initramfs

DEPENDS = $(patsubst %.o, %.d, $(OBJS))

.PHONY: all default test tester clean run run_debug
default: $(BUILD_DIR)/hug.iso
tester: $(BUILD_DIR)/libk/tester
test: tester
	$(BUILD_DIR)/libk/tester
all: default test

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
$(BUILD_DIR)/libk: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libk
$(BUILD_DIR)/libk/src: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libk/src
$(BUILD_DIR)/libk/test: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libk/test
$(BUILD_DIR)/interrupts: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/interrupts
$(BUILD_DIR)/gdt: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/gdt
$(BUILD_DIR)/threading: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/threading

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

$(BUILD_DIR)/main/%.cpp.o: src/main/%.cpp | $(BUILD_DIR)/main
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/memory/%.c.o: src/memory/%.c | $(BUILD_DIR)/memory
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/memory/%.cpp.o: src/memory/%.cpp | $(BUILD_DIR)/memory
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/interrupts/%.c.o: src/interrupts/%.c | $(BUILD_DIR)/interrupts
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/interrupts/%.cpp.o: src/interrupts/%.cpp | $(BUILD_DIR)/interrupts
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/gdt/%.c.o: src/gdt/%.c | $(BUILD_DIR)/gdt
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/gdt/%.cpp.o: src/gdt/%.cpp | $(BUILD_DIR)/gdt
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/threading/%.cpp.o: src/threading/%.cpp | $(BUILD_DIR)/threading
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/%.cpp.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/libk/src/%.c.o: src/libk/src/%.c | $(BUILD_DIR)/libk/src
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/libk/src/%.cpp.o: src/libk/src/%.cpp | $(BUILD_DIR)/libk/src
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/libk.a: $(OBJS_LIBK) | $(BUILD_DIR)
	$(AR) -rcs $@ $^

$(BUILD_DIR)/libk/src/%.c.test.o: src/libk/src/%.c | $(BUILD_DIR)/libk/src
	$(CC) $(CFLAGS_TEST) -o $@ $< -Dmalloc=hug_malloc -Dcalloc=hug_calloc -Dfree=hug_free

$(BUILD_DIR)/libk/test/%.c.o: src/libk/test/%.c | $(BUILD_DIR)/libk/test
	$(CC) $(CFLAGS_TEST) -o $@ $<

-include $(RUST_LIB_DIR)/libhugos.d
$(RUST_LIB_DIR)/libhugos.a:
ifeq ($(RUST_BUILD_TYPE),release)
	+cd $(RUST_DIR) && $(CARGO) build --release
else
	+cd $(RUST_DIR) && $(CARGO) build
endif

$(BUILD_DIR)/hug.bin: $(OBJS) $(BUILD_DIR)/libk.a $(RUST_LIB_DIR)/libhugos.a
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJS) -L$(BUILD_DIR) -lk -L$(RUST_LIB_DIR) -lhugos

$(RAMDISK_IMG): $(RAMDISK)
	$(TAR) -czf $@ $<

$(BUILD_DIR)/hug.iso: $(BUILD_DIR)/hug.bin $(GRUBCFG) $(RAMDISK_IMG) | $(ISODIR)
	cp $(BUILD_DIR)/hug.bin $(ISODIR)/boot/hug.bin
	cp $(RAMDISK_IMG) $(ISODIR)/boot/initramfs.tar.gz
	cp $(GRUBCFG) $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)
	
$(BUILD_DIR)/libk/tester: $(OBJS_LIBK_TEST) | $(BUILD_DIR)/libk
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)
	cd $(RUST_DIR) && $(CARGO) clean
run: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -drive file=$<,format=raw
run_debug: $(BUILD_DIR)/hug.iso
	$(QEMU) $(QEMU_ARGS) $(QEMU_SERIAL) -s -S -drive file=$<,format=raw
