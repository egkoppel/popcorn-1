#include "frame_bump_alloc.h"
#include <utils.h>
#include <stdio.h>

virtual_address frame_bump_alloc_allocate(frame_bump_alloc_state *self) {
	uint64_t attempt = ALIGN_UP(self->next_alloc, 0x1000);

	while (1) {
		kfprintf(stdserial, "Attempt alloc at %p\n", attempt);
		if (attempt >= self->kernel_start && attempt < self->kernel_end) {
			kfprintf(stdserial, "bump alloc kernel jump");
			attempt = ALIGN_UP(self->kernel_end, 0x1000);
			continue;
		}

		if (attempt >= self->multiboot_start && attempt < self->multiboot_end) {
			kfprintf(stdserial, "bump alloc multiboot jump");
			attempt = ALIGN_UP(self->multiboot_end, 0x1000);
			continue;
		}

		for (multiboot_memory_map_entry *entry = multiboot_tag_memory_map_begin(self->mem_map); entry < multiboot_tag_memory_map_end(self->mem_map); entry++) {
			if (entry->base_addr <= attempt && attempt < entry->base_addr + entry->length) {
				if (entry->type == AVAILABLE) {
					self->next_alloc = attempt + 0x1000;
					return (virtual_address)attempt;
				} else {
					attempt = ALIGN_UP(entry->base_addr + entry->length, 0x1000);
					break;
				}
			}
		}
		attempt = ALIGN_UP(attempt + 0x1000, 0x1000);
	}
}