#include "kernelspace_map.hpp"
#include <utils.h>
#include <stdio.h>

uint64_t KernelspaceMapper::map_to(uint64_t phys_addr, uint64_t size, entry_flags_t flags, allocator_vtable *allocator) {
	uint64_t phys_addr_start = ALIGN_DOWN(phys_addr, 0x1000);
	uint64_t phys_addr_diff = phys_addr - phys_addr_start;
	uint64_t phys_addr_end = ALIGN_UP(phys_addr + size, 0x1000);

    uint64_t virtual_addr_start = this->next_addr;

	for (; phys_addr_start < phys_addr_end; phys_addr_start += 0x1000, this->next_addr += 0x1000) {
		map_page_to(this->next_addr, phys_addr_start, flags, allocator);
        fprintf(stdserial, "Mapping %p -> %p\n", phys_addr_start, this->next_addr);
	}

    return virtual_addr_start + phys_addr_diff;
}

uint64_t KernelspaceMapper::map(uint64_t size, entry_flags_t flags, allocator_vtable *allocator) {
    uint64_t virtual_addr_start = this->next_addr;
    uint64_t needed_frame_count = IDIV_ROUND_UP(size, 0x1000);

    for (uint64_t i = 0; i < needed_frame_count; i++, this->next_addr += 0x1000) {
        map_page(this->next_addr, flags, allocator);
    }

	fprintf(stdserial, "Mapped %p\n", this->next_addr);

    return virtual_addr_start;
}