#ifndef _HUGOS_PAGING_H
#define _HUGOS_PAGING_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint64_t present: 1;
	uint64_t writable: 1;
	uint64_t user_accessible: 1;
	uint64_t write_through: 1;
	uint64_t cache_disabled: 1;
	uint64_t accessed: 1;
	uint64_t dirty: 1;
	uint64_t huge: 1;
	uint64_t global: 1;
	uint64_t available_0: 3;
	uint64_t address : 40;
	uint64_t available_1 : 11;
	uint64_t no_execute : 1;
} page_table_entry;

typedef struct __attribute__((packed)) {
	page_table_entry entries[512];
} page_table;

typedef struct {
	char valid;
	virtual_address value;
} optional_virt_address;

typedef struct {
	char valid;
	physical_address value;
} optional_physical_address;


optional_virt_address page_table_entry_get_address(page_table_entry* self);
void page_table_entry_set_address(page_table_entry* self, virtual_address address);

#endif
