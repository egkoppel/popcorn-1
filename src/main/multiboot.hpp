#ifndef _HUG_MULTIBOOT_H
#define _HUG_MULTIBOOT_H

#include <stdint.h>
#include <stdio.h>

namespace multiboot {
	enum class tag_types {
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

		struct tag_framebuffer {
			private:
			struct info_header _0;

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
			uint8_t _1;

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

			void print() {
				kprintf("Framebuffer:\n");
				kprintf("\tAddress: 0x%llx\n", addr);
				kprintf("\tPitch: %u\n", pitch);
				kprintf("\tWidth: %u\n", width);
				kprintf("\tHeight: %u\n", height);
				kprintf("\tBits per pixel: %u\n", bpp);
				kprintf("\tType: %u\n", type);
			}
		};
	}
}

class MultibootData {
	private:
	uintptr_t mb_data_start;
	uintptr_t mb_data_end;

	public:
	MultibootData(uint32_t info_struct) {
		multiboot::info_header *header = (multiboot::info_header*)info_struct;
		this->mb_data_start = info_struct + sizeof(multiboot::info_header);
		this->mb_data_end = info_struct + header->size;
	}
	~MultibootData() {}

	multiboot::tag_header* find_tag(multiboot::tag_types type);
};

#endif