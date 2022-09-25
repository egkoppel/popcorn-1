CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
OBJCOPY ?= llvm-objcopy
AR = llvm-ar
CARGO ?= cargo

INCLUDE ?= -I../libk/include -Istlport -Imemory

OPT ?= 0
WARNINGS = -Wall -Wextra -Wpedantic -Wno-language-extension-token -Werror=incompatible-pointer-types -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant
CFLAGS_TEST = $(INCLUDE_TEST) -O$(OPT) $(WARNINGS) -MMD -MP -c -g -Wno-incompatible-library-redeclaration
CFLAGS = $(INCLUDE) -O$(OPT) $(WARNINGS) -mcmodel=large -MMD -MP -c -g -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none-elf -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float
CXXFLAGS = -std=c++20
CARGOFLAGS ?=
LDFLAGS ?=

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
RUST_DIR ?= $(shell pwd)/rust

RUST_LIB_DIR = $(RUST_DIR)/target/$(RUST_TARGET_NAME)/$(RUST_BUILD_TYPE)

OBJS = $(patsubst %,$(BUILD_DIR)/%, \
	$(patsubst %.S,%.S.o,$(wildcard bootstrap/*.S)) \
	$(patsubst %.c,%.c.o,$(wildcard init/*.c)) \
	$(patsubst %.c,%.c.o,$(wildcard main/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard main/*.cpp)) \
	$(patsubst %.c,%.c.o,$(wildcard memory/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard memory/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard interrupts/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard gdt/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard threading/*.cpp)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard userspace/*.cpp)) \
	$(patsubst %.S,%.S.o,$(wildcard threading/*.S)) \
	$(patsubst %.psf,%.psf.o,$(wildcard fonts/*.psf)))

LINKER_SCRIPT ?= linker.ld

DEPENDS = $(patsubst %.o, %.d, $(OBJS))

.PHONY: all default clean
default: $(BUILD_DIR)/hug.bin
all: default

-include $(DEPENDS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
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
$(BUILD_DIR)/interrupts: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/interrupts
$(BUILD_DIR)/gdt: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/gdt
$(BUILD_DIR)/threading: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/threading
$(BUILD_DIR)/userspace: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/userspace

$(BUILD_DIR)/bootstrap/%.S.o: bootstrap/%.S | $(BUILD_DIR)/bootstrap
	$(NASM) -felf64 -g -F dwarf -o $@ $<

$(BUILD_DIR)/fonts/%.psf.o: fonts/%.psf | $(BUILD_DIR)/fonts
	$(OBJCOPY) -O elf64-x86-64 -I binary \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_start=$(subst .,_,$(lastword $(subst /, ,$<)))_start \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_end=$(subst .,_,$(lastword $(subst /, ,$<)))_end \
		--redefine-sym _binary_$(subst .,_,$(subst /,_,$<))_size=$(subst .,_,$(lastword $(subst /, ,$<)))_size \
		--rename-section .data=.rodata \
		$< $@

$(BUILD_DIR)/init/%.c.o: init/%.c | $(BUILD_DIR)/init
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/main/%.c.o: main/%.c | $(BUILD_DIR)/main
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/main/%.cpp.o: main/%.cpp | $(BUILD_DIR)/main
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/memory/%.c.o: memory/%.c | $(BUILD_DIR)/memory
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/memory/%.cpp.o: memory/%.cpp | $(BUILD_DIR)/memory
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/interrupts/%.cpp.o: interrupts/%.cpp | $(BUILD_DIR)/interrupts
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/gdt/%.cpp.o: gdt/%.cpp | $(BUILD_DIR)/gdt
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/threading/%.cpp.o: threading/%.cpp | $(BUILD_DIR)/threading
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/userspace/%.cpp.o: userspace/%.cpp | $(BUILD_DIR)/userspace
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/threading/%.S.o: threading/%.S | $(BUILD_DIR)/threading
	$(NASM) -felf64 -g -F dwarf -o $@ $<

-include $(RUST_LIB_DIR)/libhugos.d
$(RUST_LIB_DIR)/libhugos.a:
ifeq ($(RUST_BUILD_TYPE),release)
	+cd $(RUST_DIR) && $(CARGO) build --release
else
	+cd $(RUST_DIR) && $(CARGO) build
endif

$(BUILD_DIR)/hug.bin: $(OBJS) ../libk/$(BUILD_DIR)/libk.a $(RUST_LIB_DIR)/libhugos.a
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJS) -L$(RUST_LIB_DIR) -lhugos -L../libk/$(BUILD_DIR) -lk

clean:
	rm -rf $(BUILD_DIR)
	cd $(RUST_DIR) && $(CARGO) clean