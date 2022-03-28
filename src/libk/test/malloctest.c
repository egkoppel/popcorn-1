#define malloc hug_malloc
#define calloc hug_calloc
#define free hug_free
#include <malloc.h>
#undef malloc
#undef calloc
#undef free

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "sbrk.h"
#include "malloctest.h"

#define HEAPSIZE 10 * 1024 * 1024 // 10 MiB

void print_heap() {
	size_t i = 0;
	printf("Heap: first_free = %p\n", (void*)__hug_malloc_get_first_free());
	while (i < (unsigned)offset) {
		Header *header = (Header*)(heap + i);
		printf("\t%18p %8lu [ Header{free = %i, prev_free = %18p, next_free = %18p} Space{size = %8lu} Footer{header = %18p} ] ",
			(void*)header, i, header->is_free, (void*)header->prev_free, (void*)header->next_free, header->size, (void*)((Footer*)((char*)header + sizeof(Header) + header->size))->header);
		i += sizeof(Header) + header->size + sizeof(Footer);
		printf("%8lu\n", i);
	}
	printf("End Heap\n\n");
}

void test_malloc() {
	heap = malloc(HEAPSIZE);
	assert(heap != NULL);
	heapsize = HEAPSIZE;
	offset = 0;
	__hug_malloc_clear_first_free();
	__hug_malloc_set_first_malloc();
	
	printf("Beginning malloctest.\n");
	
	void *to_free[2];
	
	print_heap();
	to_free[0] = hug_malloc(0);
	void *ptr100 = hug_malloc(100);
	print_heap();
	void *ptr200 = hug_malloc(200);
	void *ptr5000 = hug_malloc(5000);
	print_heap();
	hug_free(ptr100);
	print_heap();
	hug_free(ptr5000);
	print_heap();
	void *ptr250 = hug_malloc(250);
	print_heap();
	hug_free(ptr200);
	print_heap();
	void *ptr300 = hug_malloc(300);
	print_heap();
	void *ptr400 = hug_malloc(400);
	void *ptr500 = hug_malloc(500);
	to_free[1] = hug_malloc(600);
	print_heap();
	hug_free(ptr400);
	print_heap();
	hug_free(ptr500);
	print_heap();
	hug_free(ptr300);
	print_heap();
	void *ptr64 = hug_malloc(64);
	print_heap();
	hug_free(ptr64);
	print_heap();
	hug_free(ptr250);
	print_heap();
	
	hug_free(to_free[0]);
	hug_free(to_free[1]);
	assert_msg(offset == 0, "Free did not return all memory");
	
	printf("Finished malloctest.\n");
	
	free(heap);
	heap = NULL;
	heapsize = 0;
	offset = 0;
}
