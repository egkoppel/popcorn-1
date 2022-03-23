#include "memory.h"
#include "paging.h"
#include "../main/main.h"
#include <assert.h>
#include <stdio.h>

sbrk_state_t global_sbrk_state;

void *sbrk(intptr_t increment) {
	uint64_t ret = global_sbrk_state.current_break;
	uint64_t new_break = ALIGN_UP(global_sbrk_state.current_break + increment, 0x1000);
	fprintf(stdserial, "sbrk old: %p, new: %p\n", ret, new_break);

	if (global_sbrk_state.current_break < new_break) {
		for (uint64_t page_to_map = global_sbrk_state.current_break; page_to_map < new_break; page_to_map+=0x1000) {
			map_page(page_to_map, global_frame_allocator);
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
