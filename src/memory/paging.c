#include "paging.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

