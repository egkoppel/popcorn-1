#ifndef _HUGOS_FRAME_BUMP_ALLOC_H
#define _HUGOS_FRAME_BUMP_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.hpp"
#include "memory.h"
#include "allocator.h"

struct frame_bump_alloc_state {
	allocator_vtable vtable;
	uint64_t next_alloc;
	uint64_t kernel_start;
	uint64_t kernel_end;
	uint64_t multiboot_start;
	uint64_t multiboot_end;
	multiboot::memory_map_tag *mem_map;

	static uint64_t bump_alloc_allocate(frame_bump_alloc_state *self);
	uint64_t allocate();
};

extern const allocator_vtable frame_bump_alloc_state_vtable;

#endif
