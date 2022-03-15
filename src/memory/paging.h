#ifndef _HUGOS_PAGING_H
#define _HUGOS_PAGING_H

#include <stdint.h>
#include "memory.h"
#include "allocator.h"
#include <utils.h>

int32_t map_page(uint64_t page_addr, allocator_vtable *allocator);
int32_t map_page_to(uint64_t page_addr, uint64_t frame_addr, allocator_vtable *allocator);

int32_t unmap_page(uint64_t page_addr, allocator_vtable *allocator);
void unmap_page_no_free(uint64_t page_addr);

int32_t translate_page(uint64_t page_addr, uint64_t *frame_addr);
int32_t translate_addr(uint64_t virtual_addr, uint64_t *physical_addr);

typedef struct {
	uint64_t _backup_addr;
} mapper_ctx_t;

mapper_ctx_t mapper_ctx_begin(uint64_t p4_frame_addr, allocator_vtable *allocator);
void mapper_ctx_end(mapper_ctx_t ctx);

uint64_t create_p4_table(allocator_vtable *allocator);

typedef struct {
	_Bool writeable;
	_Bool user_accessible;
	_Bool write_through;
	_Bool cache_disabled;
	_Bool accessed;
	_Bool dirty;
	_Bool huge;
	_Bool global;
	_Bool no_execute;
} entry_flags_t;

int32_t set_entry_flags_for_address(uint64_t addr, entry_flags_t flags);

#endif
