#ifndef _HUGOS_MEMORY_H
#define _HUGOS_MEMORY_H

typedef void* physical_address;
typedef void* virtual_address;

#include "allocator.h"
#include <stdint.h>

typedef struct {
	uint64_t kernel_end;
	uint64_t current_break
} sbrk_state_t;

extern sbrk_state_t global_sbrk_state;

void *sbrk(intptr_t increment); 

#endif
