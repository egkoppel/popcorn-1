#include "frame_main_alloc.hpp"
#include <stdio.h>
#include <panic.h>
#include <assert.h>

const allocator_vtable frame_main_alloc_state_vtable = {
	.allocate = reinterpret_cast<uint64_t(*)(struct _allocator_vtable*)>(frame_main_alloc_state::main_alloc_allocate),
	.deallocate =  reinterpret_cast<void(*)(struct _allocator_vtable*, uint64_t)>(frame_main_alloc_state::main_alloc_deallocate)
};

int frame_main_alloc_state::set_bit(uint64_t addr) {
	uint64_t frame_num = (uint64_t)addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;

	fprintf(stdserial, "Set allocator bit for %p (%llx, %llx, %llx)\n", addr, frame_num, bitmap_index, bit_index);

	if(this->bitmap_start + bitmap_index < this->bitmap_end) {
		this->bitmap_start[bitmap_index] |= (1 << bit_index);
		return 0;
	}
	return -1;
}

uint64_t frame_main_alloc_state::allocate() {
	for (uint64_t *addr = this->bitmap_start; addr < this->bitmap_end; addr++) {
		fprintf(stdserial, "checking bits: %064llb (%llu)\n", *addr, addr - this->bitmap_start);
		if (*addr != UINT64_MAX) {
			uint64_t first_clear_bit = __builtin_ffsll(~(*addr));
			fprintf(stdserial, "first clear %llu\n", first_clear_bit);
			*addr |= (1ull << (first_clear_bit - 1));
			auto ret = ((uint64_t)addr - (uint64_t)this->bitmap_start + first_clear_bit - 1)*0x1000;
			fprintf(stdserial, "Allocated frame %p\n", ret);
			return ret;
		} else {
			fprintf(stdserial, "full\n");
		}
	}
	panic("OOOOOOOOOOM");
}

void frame_main_alloc_state::deallocate(uint64_t addr) {
	fprintf(stdserial, "Deallocate frame at %p\n", addr);
	uint64_t frame_num = addr / 0x1000;
	uint64_t bitmap_index = frame_num / 64;
	uint64_t bit_index = frame_num % 64;
	assert_msg(this->bitmap_start + bitmap_index < this->bitmap_end, "Attempted deallocation at %p outside of bitmap", addr);
	this->bitmap_start[bitmap_index] &= ~(1ull << bit_index);
}

uint64_t frame_main_alloc_state::main_alloc_allocate(frame_main_alloc_state* allocator) {
	return allocator->allocate();
}

void frame_main_alloc_state::main_alloc_deallocate(frame_main_alloc_state* allocator, uint64_t addr) {
	allocator->deallocate(addr);
}

uint64_t frame_main_alloc_state::free_frame_count() {
	uint64_t ret = 0;
	for (uint64_t *addr = this->bitmap_start; addr < this->bitmap_end; addr++) {
		ret += __builtin_popcountll(~(*addr));
	}
	return ret;
}

uint64_t frame_main_alloc_state::used_frame_count() {
	uint64_t ret = 0;
	for (uint64_t *addr = this->bitmap_start; addr < this->bitmap_end; addr++) {
		ret += __builtin_popcountll(*addr);
	}
	return ret;
}
