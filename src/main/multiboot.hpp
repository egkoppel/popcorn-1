#ifndef _HUG_MULTIBOOT_H
#define _HUG_MULTIBOOT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils.h>

namespace multiboot {
	enum class tag_type: uint32_t {
		CLI = 1,
		BOOTLOADER_NAME = 2,
		BOOT_MODULE = 3,
		MEMORY_MAP = 6,
		FRAMEBUFFER = 8,
		ELF_SECTIONS = 9
	};

	struct __attribute__((packed)) info_header {
		uint32_t size;
		uint32_t reserved;
	};

	struct __attribute__((packed)) tag_header {
		tag_type type;
		uint32_t size;
	};

	struct __attribute__((packed)) cli_tag {
		tag_header header;
		char str;

		const char* get_str();
	};

	struct __attribute__((packed)) bootloader_tag {
		tag_header header;
		char str;

		const char* get_name();
	};

	enum class memory_type: uint32_t {
		RESERVED = 0,
		AVAILABLE = 1,
		ACPI = 3,
		HIBERNATION_SAVE = 4,
		DEFECTIVE = 5
	};

	struct __attribute__((packed)) memory_map_entry {
		uint64_t base_addr;
		uint64_t length;
		memory_type type;
		uint32_t reserved;

		bool operator<=>(const memory_map_entry&) const = default;
		bool operator!=(const memory_map_entry&) const = default;
	};

	struct __attribute__((packed)) memory_map_tag {
		tag_header header;
		uint32_t entry_size;
		uint32_t entry_version;
		memory_map_entry first_entry;

		memory_map_entry* begin();
		memory_map_entry* end();
	};

	struct __attribute__((packed)) boot_module_tag {
		tag_header header;
		uint32_t module_start;
		uint32_t module_end;
		char str;

		const char* get_name();
	};

	struct __attribute__((packed)) framebuffer_tag {
		tag_header header;
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
	};

	struct __attribute__((packed)) elf_sections_entry {
		uint32_t name_index;
		uint32_t type;
		uint64_t flags;
		uint64_t addr;
		uint64_t offset;
		uint64_t size;
		uint32_t link;
		uint32_t info;
		uint64_t align;
		uint64_t entry_size;

		void print();
		bool operator<=>(const elf_sections_entry&) const = default;
		bool operator!=(const elf_sections_entry&) const = default;
	};

	struct __attribute__((packed)) elf_sections_tag {
		tag_header header;
		uint32_t entry_count;
		uint32_t entry_size;
		uint32_t string_table;
		elf_sections_entry first_entry;

		elf_sections_entry* begin();
		elf_sections_entry* end();
	};

	class Data {
		public:
		tag_header *mb_data_start;
		tag_header *mb_data_end;

		public:
		Data(uint64_t info_struct);
		template<class T> T* find_tag(tag_type type) {
			tag_header *current_tag = this->mb_data_start;
			while (current_tag < this->mb_data_end) {
				if (current_tag->type == type) {
					return reinterpret_cast<T*>(current_tag);
				}
				current_tag = ADD_BYTES(current_tag, current_tag->size);
				current_tag = ALIGN_UP(current_tag, 8);
			}
			return nullptr;
		}
	};
}

enum multiboot_tag_elf_symbols_entry_type {
	SHT_NULL,
	SHT_PROGBITS,
	SHT_SYMTAB,
	SHT_STRTAB,
	SHT_RELA,
	SHT_HASH,
	SHT_DYNAMIC,
	SHT_NOTE,
	SHT_NOBITS,
	SHT_REL,
	SHT_SHLIB,
	SHT_DYNSYM,
	SHT_INIT_ARRAY,
	SHT_FINI_ARRAY,
	SHT_PREINIT_ARRAY,
	SHT_GROUP,
	SHT_SYMTAB_SHNDX,
	SHT_NUM
};

constexpr uint64_t SHF_WRITE = 0x1;
constexpr uint64_t SHF_ALLOC = 0x2;
constexpr uint64_t SHF_EXECINSTR = 0x4;
constexpr uint64_t SHF_MERGE = 0x10;
constexpr uint64_t SHF_STRINGS = 0x20;
constexpr uint64_t SHF_INFO_LINK = 0x40;
constexpr uint64_t SHF_LINK_ORDER = 0x80;
constexpr uint64_t SHF_OS_NONCONFORMING = 0x100;
constexpr uint64_t SHF_GROUP = 0x200;
constexpr uint64_t SHF_TLS = 0x400;
constexpr uint64_t SHF_MASKOS = 0x0ff00000;
constexpr uint64_t SHF_MASKPROC = 0xf0000000;

#endif
