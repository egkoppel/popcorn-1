#include "frame_main_alloc.h"
#include <utils.h>
#include <stdio.h>
#include <panic.h>
#include <assert.h>

const allocator_vtable frame_main_alloc_state_vtable = {
	.allocate = (void*(*)(struct _allocator_vtable*))frame_main_alloc_allocate,
	.deallocate =  (void(*)(struct _allocator_vtable*, void*))frame_main_alloc_deallocate
};

int frame_main_alloc_set_bit_for_addr(frame_main_alloc_state *self, uint64_t addr) {
	uint64_t frame_num = (uint64_t)addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;

	if(self->bitmap_start + bitmap_index < self->bitmap_end) {
		self->bitmap_start[bitmap_index] |= (1 << bit_index);
		return 0;
	}
	return -1;
}

virtual_address frame_main_alloc_allocate(frame_main_alloc_state *self) {
	for (uint64_t *addr = self->bitmap_start; addr < self->bitmap_end; addr++) {
		if (*addr != UINT64_MAX) {
			uint64_t first_clear_bit = __builtin_ffsll(~(*addr));
			*addr |= (1 << (first_clear_bit - 1));
			return (virtual_address)(((uint64_t)addr - (uint64_t)self->bitmap_start + first_clear_bit - 1)*0x1000);
		}
	}
	panic("OOOOOOOOOOM");
}

void frame_main_alloc_deallocate(frame_main_alloc_state *self, virtual_address addr) {
	uint64_t frame_num = (uint64_t)addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;
	assert(self->bitmap_start + bitmap_index < self->bitmap_end, "Attempted deallocation at %p outside of bitmap", addr);
	self->bitmap_start[bitmap_index] &= ~(1 << bit_index);
}
