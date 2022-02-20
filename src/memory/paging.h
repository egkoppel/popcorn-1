#ifndef _HUGOS_PAGING_H
#define _HUGOS_PAGING_H

#include <stdint.h>
#include "memory.h"
#include "allocator.h"
#include <utils.h>

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

typedef struct {
	uint8_t writable: 1;
	uint8_t user_accessible: 1;
	uint8_t write_through: 1;
	uint8_t cache_disabled: 1;
	uint8_t global: 1;
	uint8_t no_execute : 1;
} page_table_entry_flags;

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

extern page_table *p4;

optional_virt_address page_table_entry_get_address(page_table_entry* self);
void page_table_entry_set(page_table_entry* self, virtual_address address, page_table_entry_flags flags);
void page_table_entry_set_flags(page_table_entry* self, page_table_entry_flags flags);
void page_table_entry_set_address(page_table_entry* self, virtual_address address);

void page_table_clear(page_table* self);
page_table* page_table_get_child(page_table* self, uintptr_t i);
page_table* page_table_create_child(page_table* self, uintptr_t i, allocator_vtable *allocator, page_table_entry_flags flags);
optional_physical_address translate_address(virtual_address address);

void map_page(virtual_address page, allocator_vtable *allocator, page_table_entry_flags flags);
void unmap_page(virtual_address page);
void unmap_page_and_free_frame(virtual_address page, allocator_vtable *allocator);
void map_address(virtual_address address, allocator_vtable *allocator, page_table_entry_flags flags);
void map_page_to(virtual_address page, physical_address frame, allocator_vtable *allocator, page_table_entry_flags flags);

#endif
