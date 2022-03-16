#include "multiboot.h"
#include <utils.h>
#include <stdio.h>

const char* multiboot_tag_cli_get_str(multiboot_tag_cli* self) {
	return &self->str;
}

const char* multiboot_tag_bootloader_get_name(multiboot_tag_bootloader* self) {
	return &self->str;
}

multiboot_memory_map_entry* multiboot_tag_memory_map_begin(multiboot_tag_memory_map* self) {
	return &self->first_entry;
}

multiboot_memory_map_entry* multiboot_tag_memory_map_end(multiboot_tag_memory_map* self) {
	return (multiboot_memory_map_entry*)(ADD_BYTES(self, self->header.size));
}

void multiboot_elf_symbols_entry_print(multiboot_elf_symbols_entry* self) {
	kprintf("\tphys: %lp virt: %lp S: 0x%llx F: 0x%llx\n", self->addr - 0xFFFFFF8000000000, self->addr, self->size, self->flags);
}

multiboot_elf_symbols_entry* multiboot_tag_elf_symbols_begin(multiboot_tag_elf_symbols* self) {
	return &self->first_entry;
}

multiboot_elf_symbols_entry* multiboot_tag_elf_symbols_end(multiboot_tag_elf_symbols* self) {
	return ADD_BYTES(&self->first_entry, self->entry_count * self->entry_size);
}

void multiboot_data_init(multiboot_data* self, uint64_t info_struct) {
	multiboot_info_header *header = (multiboot_info_header*)info_struct;
	self->mb_data_start = (multiboot_tag_header*)(info_struct + sizeof(multiboot_info_header));
	self->mb_data_end = (multiboot_tag_header*)(info_struct + header->size);
}

multiboot_tag_header* multiboot_data_find_tag(multiboot_data* self, multiboot_tag_types type) {
	multiboot_tag_header *current_tag = self->mb_data_start;
	while (current_tag < self->mb_data_end) {
		if (current_tag->type == type) {
			return current_tag;
		}
		current_tag = ADD_BYTES(current_tag, current_tag->size);
		current_tag = ALIGN_UP(current_tag, 8);
	}
	return NULL;
}

void multiboot_data_print_tags(multiboot_data* self) {
	multiboot_tag_header *current_tag = self->mb_data_start;
	while (current_tag < self->mb_data_end) {
		kprintf("Tag type: %u, size: 0x%x\n", current_tag->type, current_tag->size);
		current_tag = ADD_BYTES(current_tag, current_tag->size);
		current_tag = ALIGN_UP(current_tag, 8);
	}
}
