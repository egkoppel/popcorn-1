#include "paging.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

page_table *p4 = (page_table*)01777777767767767760000;

optional_virt_address page_table_entry_get_address(page_table_entry* self) {
	optional_virt_address result;
	result.valid = self->present;
	result.value = (void*)(self->address << 12);
	return result;
}

void page_table_entry_set_address(page_table_entry* self, virtual_address address) {
	assert(((uintptr_t)address & ~0x000ffffffffff000) == 0, "Frame address is not 4k aligned");
	self->present = 1;
	self->address = (uintptr_t)address >> 12;
}

void page_table_entry_set_flags(page_table_entry* self, page_table_entry_flags flags) {
	self->writable = 1;
	self->user_accessible = 1;
	self->write_through = flags.write_through;
	self->cache_disabled = flags.cache_disabled;
	self->global = flags.global;
	self->no_execute = flags.no_execute;
}

void page_table_entry_set(page_table_entry* self, virtual_address address, page_table_entry_flags flags) {
	page_table_entry_set_address(self, address);
	page_table_entry_set_flags(self, flags);
}

void page_table_clear(page_table* self) {
	for (int i = 0; i < 512; i++) {
		self->entries[i].present = 0;
	}
}

page_table* page_table_get_child(page_table* self, uintptr_t i) {
	assert(i < 512, "Index out of bounds");
	if (self->entries[i].present && !self->entries[i].huge) {
		return (page_table*)(((uintptr_t)self << 9) | ((uintptr_t)i << 12) | 01777770000000000000000);
	} else return NULL;
}

static inline uint64_t address_p4_index(virtual_address address) {
	return ((uint64_t)address >> 39) & 0777;
}

static inline uint64_t address_p3_index(virtual_address address) {
	return ((uint64_t)address >> 30) & 0777;
}

static inline uint64_t address_p2_index(virtual_address address) {
	return ((uint64_t)address >> 21) & 0777;
}

static inline uint64_t address_p1_index(virtual_address address) {
	return ((uint64_t)address >> 12) & 0777;
}

static inline uint64_t address_offset(virtual_address address) {
	return ((uint64_t)address) & 07777;
}

optional_physical_address translate_address(virtual_address address) {
	optional_physical_address result;
	uint64_t p4_index = address_p4_index(address);
	uint64_t p3_index = address_p3_index(address);
	uint64_t p2_index = address_p2_index(address);
	uint64_t p1_index = address_p1_index(address);
	uint64_t offset = address_offset(address);

	page_table *p3 = page_table_get_child(p4, p4_index);
	if (p3 == NULL) {
		result.valid = 0;
		return result;
	}

	page_table *p2 = page_table_get_child(p3, p3_index);
	if (p2 == NULL) {
		result.valid = 0;
		return result;
	}

	page_table *p1 = page_table_get_child(p2, p2_index);
	if (p1 == NULL) {
		if (p2->entries[p2_index].huge) {
			result.valid = 1;
			result.value = (void*)((p2->entries[p2_index].address << 12) + (p1_index << 12) + offset);
			return result;
		} else {
			result.valid = 0;
			return result;
		}
	}

	page_table_entry *p1_entry = &p1->entries[p1_index];
	if (!p1_entry->present) {
		result.valid = 0;
		return result;
	}
	result.valid = 1;
	result.value = (void*)((p1_entry->address << 12) + offset);

	return result;
}
