#ifndef _HUGOS_FRAME_BUMP_ALLOC_H
#define _HUGOS_FRAME_BUMP_ALLOC_H

#include <stdint.h>
#include "../main/multiboot.h"
#include "memory.h"

typedef struct {
	uint64_t next_alloc;
	uint64_t kernel_start;
	uint64_t kernel_end;
	uint64_t multiboot_start;
	uint64_t multiboot_end;
	multiboot_tag_memory_map *mem_map;
} frame_bump_alloc_state;

virtual_address frame_bump_alloc_allocate(frame_bump_alloc_state *self);

#endif