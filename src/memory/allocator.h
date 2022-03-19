#ifndef _HUGOS_ALLOCATOR_H
#define _HUGOS_ALLOCATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _allocator_vtable {
	uint64_t (*allocate)(struct _allocator_vtable*);
	void (*deallocate)(struct _allocator_vtable*, uint64_t);
} allocator_vtable;

uint64_t allocator_allocate(allocator_vtable*);
void allocator_deallocate(allocator_vtable*, uint64_t);

#ifdef __cplusplus
}
#endif

#endif
