#ifndef _HUGOS_FRAME_MAIN_ALLOC_H
#define _HUGOS_FRAME_MAIN_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.h"
#include "memory.h"
#include "allocator.h"

typedef struct {
	allocator_vtable vtable;
	uint64_t *bitmap_start;
	uint64_t *bitmap_end;
} frame_main_alloc_state;

extern const allocator_vtable frame_main_alloc_state_vtable;

int frame_main_alloc_set_bit_for_addr(frame_main_alloc_state *self, uint64_t addr);
virtual_address frame_main_alloc_allocate(frame_main_alloc_state *self);
void frame_main_alloc_deallocate(frame_main_alloc_state *self, virtual_address addr);

#endif
