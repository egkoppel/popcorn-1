#include "stack.hpp"
#include "../main/main.h"
#include "allocator.h"
#include <assert.h>
#include "paging.h"
#include <stdio.h>

uint64_t stack_next_alloc = 0177777'775'777'777'777'7777;

Stack::Stack(uint64_t size,  bool user_access) {
	fprintf(stdserial, "Allocating new stack of size %llx\n", size);
	assert_msg(size > 0, "Cannot have zero sized stack");
	assert_msg(0 == (size & (0x1000 - 1)), "Stack size must be a multiple of 4k");
	this->top = stack_next_alloc;
	this->bottom = stack_next_alloc - size + 1;
	
	entry_flags_t flags = {
		.writeable = true,
		.user_accessible = user_access,
		.write_through = false,
		.cache_disabled = false,
		.accessed = false,
		.dirty = false,
		.huge = false,
		.global = false,
		.no_execute = true,
	};

	fprintf(stdserial, "Creating new stack: %p -> %p - Guard at %p\n", this->bottom, this->top, this->bottom - 0x1000);

	for (uint64_t stack_addr = this->bottom; stack_addr < this->top; stack_addr += 0x1000) {
		map_page(stack_addr, flags, global_frame_allocator);
	}
	
	mark_for_no_map(this->bottom - 0x1000, global_frame_allocator); // Guard page

	stack_next_alloc = this->bottom - 0x1000 - 1;
}

Stack::~Stack() {
	fprintf(stdserial, "Stack (%p -> %p) dropped\n", this->bottom, this->top);
	for (uint64_t stack_addr = this->bottom; stack_addr < this->top; stack_addr += 0x1000) {
		unmap_page(stack_addr, global_frame_allocator);
	}
	
	unmark_for_no_map(this->bottom - 0x1000); // Guard page
}
