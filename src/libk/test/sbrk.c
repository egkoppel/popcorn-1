#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include "sbrk.h"

char *heap;
intptr_t offset;
size_t heapsize;

void *sbrk(intptr_t increment) {
	intptr_t old_offset = offset;
	offset += increment;
	assert((unsigned)offset < heapsize);
	return (void*)(heap + old_offset);
}
