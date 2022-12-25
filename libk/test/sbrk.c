#include "sbrk.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

char *heap;
intptr_t offset;
size_t heapsize;

void *sbrk(intptr_t increment) {
	intptr_t old_offset = offset;
	intptr_t new_offset = offset + increment;
	if ((unsigned)new_offset >= heapsize) return (void *)-1;
	if (new_offset < 0) return (void *)-1;
	offset = new_offset;
	return (void *)(heap + old_offset);
}
