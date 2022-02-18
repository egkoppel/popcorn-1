#ifndef _HUGOS_ALLOCATOR_H
#define _HUGOS_ALLOCATOR_H

typedef struct _allocator_vtable {
	void* (*allocate)(struct _allocator_vtable*);
	void (*deallocate)(struct _allocator_vtable*, void*);
} allocator_vtable;

void* allocator_allocate(allocator_vtable*);
void allocator_deallocate(allocator_vtable*, void*);

#endif