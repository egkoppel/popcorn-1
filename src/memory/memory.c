#include "memory.h"
#include "paging.h"
#include "../main/main.h"
#include <assert.h>
#include <stdio.h>

sbrk_state_t global_sbrk_state = {
	.initialised = false
};

void *sbrk(intptr_t increment) {
	assert_msg(global_sbrk_state.initialised, "sbrk() used before initialisation");
	
	uint64_t ret = global_sbrk_state.current_break;
	uint64_t new_break = ALIGN_UP(global_sbrk_state.current_break + increment, 0x1000);
	fprintf(stdserial, "sbrk old: %p, new: %p\n", ret, new_break);

	if (global_sbrk_state.current_break < new_break) {
		for (uint64_t page_to_map = global_sbrk_state.current_break; page_to_map < new_break; page_to_map+=0x1000) {
			entry_flags_t flags = {
				.writeable = 1,
				.user_accessible = 0,
				.write_through = 0,
				.cache_disabled = 0,
				.accessed = 0,
				.dirty = 0,
				.huge = 0,
				.global = 0,
				.no_execute = 1
			};
			map_page(page_to_map, flags, global_frame_allocator);
		}
	} else if (global_sbrk_state.current_break > new_break) {
		assert_msg(new_break > global_sbrk_state.kernel_end, "Attempt to unnmap kernel with sbrk");
		for (uint64_t page_to_unmap = new_break; page_to_unmap < global_sbrk_state.current_break; page_to_unmap+=0x1000) {
			unmap_page(page_to_unmap, global_frame_allocator);
		}
	}
	global_sbrk_state.current_break = new_break;

	return ret;
}
