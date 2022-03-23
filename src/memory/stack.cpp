#include "stack.hpp"
#include "../main/main.h"
#include "allocator.h"
#include <assert.h>
#include "paging.h"
#include <stdio.h>

uint64_t stack_next_alloc = 0177777'775'777'777'777'7777;

Stack::Stack(uint64_t size) {
	fprintf(stdserial, "Allocating new stack of size %llx\n", size);
	assert_msg(size > 0, "Cannot have zero sized stack");
	assert_msg(0 == (size & (0x1000 - 1)), "Stack size must be a multiple of 4k");
	this->top = stack_next_alloc;
	this->bottom = stack_next_alloc - size + 1;
	
	entry_flags_t flags = {
		.accessed = 0,
		.cache_disabled = 0,
		.dirty = 0,
		.global = 0,
		.huge = 0,
		.no_execute = 1,
		.user_accessible = 0,
		.write_through = 0,
		.writeable = 1
	};

	fprintf(stdserial, "Creating new stack: %p -> %p - Guard at %p\n", this->bottom, this->top, this->bottom - 0x1000);

	for (uint64_t stack_addr = this->bottom; stack_addr < this->top; stack_addr += 0x1000) {
		map_page(stack_addr, global_frame_allocator);
		set_entry_flags_for_address(stack_addr, flags);
	}
	int guard_page_mapped = translate_page(this->bottom - 0x1000, NULL);
	assert_msg(guard_page_mapped == -1, "Stack guard page already mapped");

	stack_next_alloc = this->bottom - 0x1000 - 1;
}
