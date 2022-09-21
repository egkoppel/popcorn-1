#ifndef _HUGOS_KERNELSPACE_MAP_H
#define _HUGOS_KERNELSPACE_MAP_H

#include <utils.h>
#include "paging.h"

class KernelspaceMapper {
	private:
	uint64_t next_addr;

	public:
	KernelspaceMapper(uint64_t start_addr) : next_addr(ALIGN_UP(start_addr, 0x1000)) {};

	uint64_t map_to(uint64_t phys_addr, uint64_t size, entry_flags_t flags, allocator_vtable *allocator);
    uint64_t map(uint64_t size, entry_flags_t flags, allocator_vtable *allocator);
};

#endif
