CC ?= clang
CXX ?= clang++
NASM ?= nasm
LD = ld.lld
AR = llvm-ar

BUILD_DIR ?= build

INCLUDE ?= -Iinclude
INCLUDE_TEST ?= -Iinclude

OPT ?= 0
WARNINGS = -Wall -Wextra -Wpedantic -Wno-language-extension-token -Werror=incompatible-pointer-types -Wno-address-of-packed-member -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-folding-constant
CFLAGS_TEST = $(INCLUDE_TEST) -O$(OPT) $(WARNINGS) -MMD -MP -c -g -Wno-incompatible-library-redeclaration
CFLAGS = $(INCLUDE) -O$(OPT) $(WARNINGS) -mcmodel=large -MMD -MP -c -g -nostdlib -fno-exceptions -fno-rtti -fno-stack-protector -ffreestanding -target x86_64-unknown-none-elf -mno-mmx -mno-sse -mno-sse3 -mno-sse4 -mno-avx -mno-red-zone -msoft-float
CFLAGS_RAMFS = -O$(OPT) $(WARNINGS) -c -ffreestanding -target x86_64-unknown-none-elf
CXXFLAGS = -std=c++20

OBJS_LIBK = $(patsubst %,$(BUILD_DIR)/%, \
	$(patsubst %.c,%.c.o,$(wildcard src/*.c)) \
	$(patsubst %.cpp,%.cpp.o,$(wildcard src/*.cpp)))
OBJS_LIBK_TEST = $(patsubst %,$(BUILD_DIR)/%, \
	$(patsubst %.c,%.c.test.o, src/malloc.c) \
	$(patsubst %.c,%.c.o,$(wildcard test/*.c)))

.PHONY: all default test tester clean
default: $(BUILD_DIR)/libk.a
tester: $(BUILD_DIR)/libk/tester

test: tester
	$(BUILD_DIR)/libk/tester

all: default tester

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BUILD_DIR)/src: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/src
$(BUILD_DIR)/test: | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/test

$(BUILD_DIR)/src/%.c.o: src/%.c | $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/src/%.cpp.o: src/%.cpp | $(BUILD_DIR)/src
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/libk.a: $(OBJS_LIBK) | $(BUILD_DIR)
	$(AR) -rcs $@ $^

$(BUILD_DIR)/src/%.c.test.o: src/%.c | $(BUILD_DIR)/src
	$(CC) $(CFLAGS_TEST) -o $@ $< -Dmalloc=hug_malloc -Dcalloc=hug_calloc -Drealloc=hug_realloc -Dfree=hug_free

$(BUILD_DIR)/test/%.c.o: test/%.c | $(BUILD_DIR)/test
	$(CC) $(CFLAGS_TEST) -o $@ $<

$(BUILD_DIR)/tester: $(OBJS_LIBK_TEST) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^