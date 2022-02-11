#ifndef _HUG_MULTIBOOT_H
#define _HUG_MULTIBOOT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils.hpp>

namespace multiboot {
	enum class tag_types {
		CLI = 1,
		BOOTLOADER_NAME = 2,
		MEMORY_MAP = 6,
		FRAMEBUFFER = 8
	};

	extern "C" {
		struct info_header {
			uint32_t size;
			uint32_t reserved;
		};

		struct tag_header {
			uint32_t type;
			uint32_t size;
		};

		struct tag_cli {
			private:
			tag_header header;
			char str;

			public:
			const char* get_str() {
				return &str;
			}
		};

		struct tag_bootloader {
			private:
			tag_header header;
			char str;

			public:
			const char* get_name() {
				return &str;
			}
		};

		struct memory_map_entry {
			uint64_t base_addr;
			uint64_t length;
			enum: uint32_t {
				RESERVED = 0,
				AVAILABLE = 1,
				ACPI = 3,
				HIBERNATION_SAVE = 4,
				DEFECTIVE = 5
			} type;
			
			private:
			uint32_t reserved;
		};

		struct tag_memory_map {
			private:
			tag_header header;
			uint32_t entry_size;
			uint32_t entry_version;
			memory_map_entry first_entry;

			public:
			memory_map_entry* begin() {
				return &first_entry;
			}
			memory_map_entry* end() {
				return reinterpret_cast<memory_map_entry*>(add_bytes(this, this->header.size));//(memory_map_entry*)((uintptr_t)this + this->header.size);
			}
		};

		struct tag_framebuffer {
			private:
			struct tag_header header;

			public:
			uint64_t addr;
			uint32_t pitch;
			uint32_t width;
			uint32_t height;
			uint8_t bpp;
			enum: uint8_t {
				INDEXED = 0,
				RGB = 1,
				EGA_TEXT = 2
			} type;

			private:
			uint8_t _0;

			public:
			union {
				struct {
					uint32_t num_colors;
				} indexed;

				struct {
					uint8_t red_pos;
					uint8_t red_mask_size;
					uint8_t green_pos;
					uint8_t green_mask_size;
					uint8_t blue_pos;
					uint8_t blue_mask_size;
				} rgb;
			} color_info;
		};
	}
}

class MultibootData {
	private:
	multiboot::tag_header *mb_data_start;
	multiboot::tag_header *mb_data_end;

	public:
	MultibootData(uint32_t info_struct) {
		multiboot::info_header *header = reinterpret_cast<multiboot::info_header*>(info_struct);
		this->mb_data_start = reinterpret_cast<multiboot::tag_header*>(info_struct + sizeof(multiboot::info_header));
		this->mb_data_end = reinterpret_cast<multiboot::tag_header*>(info_struct + header->size);
	}
	~MultibootData() {}

	typedef union {
		multiboot::tag_header* generic;
		multiboot::tag_cli* cli;
		multiboot::tag_bootloader* bootloader;
		multiboot::tag_memory_map* memory_map;
		multiboot::tag_framebuffer* framebuffer;
	} tag_union;
	tag_union find_tag(multiboot::tag_types type);
	void print_tags();
};

#endif