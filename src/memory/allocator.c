#include "allocator.h"
#include <stddef.h>

uint64_t allocator_allocate(allocator_vtable *allocator) {
	if (allocator->allocate != NULL) return allocator->allocate(allocator);
	return 0;
}

void allocator_deallocate(allocator_vtable *allocator, uint64_t address) {
	if (allocator->deallocate != NULL) allocator->deallocate(allocator, address);
}
