#include "allocator.h"
#include <stddef.h>

void* allocator_allocate(allocator_vtable *allocator) {
	if (allocator->allocate != NULL) return allocator->allocate(allocator);
	return NULL;
}

void allocator_deallocate(allocator_vtable *allocator, void *address) {
	if (allocator->deallocate != NULL) allocator->deallocate(allocator, address);
}
