#ifndef _HUGOS_FRAME_MAIN_ALLOC_H
#define _HUGOS_FRAME_MAIN_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.hxx"
#include "memory.h"
#include "allocator.h"

struct frame_main_alloc_state {
	allocator_vtable vtable;
	uint64_t *bitmap_start;
	uint64_t *bitmap_end;

	int set_bit(uint64_t addr);
	uint64_t allocate();
	void deallocate(uint64_t addr);

	static uint64_t main_alloc_allocate(frame_main_alloc_state*);
	static void main_alloc_deallocate(frame_main_alloc_state*, uint64_t addr);
};

extern const allocator_vtable frame_main_alloc_state_vtable;

#endif
