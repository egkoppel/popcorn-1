#include "paging.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

page_table *p4 = (page_table*)01777777767767767760000;

optional_virt_address page_table_entry_get_address(page_table_entry* self) {
	optional_virt_address result;
	result.valid = self->present;
	result.value = self->address << 12;
	return result;
}

void page_table_entry_set_address(page_table_entry* self, virtual_address address) {
	assert(((uintptr_t)address & ~0x000ffffffffff000) == 0, "Frame address is not 4k aligned");
	self->present = 1;
	self->address = (uintptr_t)address >> 12;
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

