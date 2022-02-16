#ifndef _HUG_MULTIBOOT_H
#define _HUG_MULTIBOOT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils.h>

typedef enum {
	CLI = 1,
	BOOTLOADER_NAME = 2,
	MEMORY_MAP = 6,
	FRAMEBUFFER = 8
} multiboot_tag_types;

typedef struct __attribute__((packed)) {
	uint32_t size;
	uint32_t reserved;
} multiboot_info_header;

typedef struct __attribute__((packed)) {
	uint32_t type;
	uint32_t size;
} multiboot_tag_header;

typedef struct __attribute__((packed)) {
	multiboot_tag_header header;
	char str;
} multiboot_tag_cli;

const char* multiboot_tag_cli_get_str(multiboot_tag_cli* self);

typedef struct __attribute__((packed)) {
	multiboot_tag_header header;
	char str;
} multiboot_tag_bootloader;

const char* multiboot_tag_bootloader_get_name(multiboot_tag_bootloader* self);

typedef struct __attribute__((packed)) {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} multiboot_memory_map_entry;

typedef enum {
	RESERVED = 0,
	AVAILABLE = 1,
	ACPI = 3,
	HIBERNATION_SAVE = 4,
	DEFECTIVE = 5
} multiboot_memory_map_entry_type;

typedef struct __attribute__((packed)) {
	multiboot_tag_header header;
	uint32_t entry_size;
	uint32_t entry_version;
	multiboot_memory_map_entry first_entry;
} multiboot_tag_memory_map;

multiboot_memory_map_entry* multiboot_tag_memory_map_begin(multiboot_tag_memory_map* self);
multiboot_memory_map_entry* multiboot_tag_memory_map_end(multiboot_tag_memory_map* self);

typedef struct __attribute__((packed)) {
	multiboot_tag_header header;
	uint64_t addr;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t type;

	uint8_t _0;

	union {
		struct __attribute__((packed)) {
			uint32_t num_colors;
		} indexed;

		struct __attribute__((packed)) {
			uint8_t red_pos;
			uint8_t red_mask_size;
			uint8_t green_pos;
			uint8_t green_mask_size;
			uint8_t blue_pos;
			uint8_t blue_mask_size;
		} rgb;
	} color_info;
} multiboot_tag_framebuffer;

typedef struct __attribute__((packed)) {
	multiboot_tag_header *mb_data_start;
	multiboot_tag_header *mb_data_end;
} multiboot_data;

void multiboot_data_init(multiboot_data* self, uint32_t info_struct);
multiboot_tag_header* multiboot_data_find_tag(multiboot_data* sef, multiboot_tag_types type);
void multiboot_data_print_tags(multiboot_data* self);

#endif
