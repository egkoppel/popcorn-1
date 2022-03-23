#include "multiboot.hxx"
#include <utils.h>
#include <stdio.h>

using namespace multiboot;

const char* cli_tag::get_str() {
	return &this->str;
}

const char* bootloader_tag::get_name() {
	return &this->str;
}

memory_map_entry* memory_map_tag::begin() {
	return &this->first_entry;
}

memory_map_entry* memory_map_tag::end() {
	return reinterpret_cast<memory_map_entry*>(ADD_BYTES(this, this->header.size));
}

void elf_sections_entry::print() {
	printf("\tphys: %lp virt: %lp S: 0x%llx F: 0x%llx\n", this->addr - 0xFFFFFF8000000000, this->addr, this->size, this->flags);
}

elf_sections_entry* elf_sections_tag::begin() {
	return &this->first_entry;
}

elf_sections_entry* elf_sections_tag::end() {
	return reinterpret_cast<elf_sections_entry*>(ADD_BYTES(&this->first_entry, this->entry_count * this->entry_size));
}

const char* boot_module_tag::get_name() {
	return &this->str;
}

Data::Data(uint64_t info_struct) {
	info_header *header = reinterpret_cast<info_header*>(info_struct);
	this->mb_data_start = reinterpret_cast<tag_header*>(info_struct + sizeof(info_header));
	this->mb_data_end = reinterpret_cast<tag_header*>(info_struct + header->size);
}
