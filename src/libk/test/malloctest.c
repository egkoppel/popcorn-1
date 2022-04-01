#define malloc hug_malloc
#define calloc hug_calloc
#define realloc hug_realloc
#define free hug_free
#include <malloc.h>
#undef malloc
#undef calloc
#undef realloc
#undef free

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "sbrk.h"
#include "malloctest.h"

#define HEAPSIZE 10 * 1024 * 1024 // 10 MiB

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
	
	void *ptr150 = hug_malloc(150);
	print_heap();
	ptr150 = hug_realloc(ptr150, 200);
	print_heap();
	ptr500 = hug_malloc(500);
	ptr300 = hug_malloc(300);
	print_heap();
	ptr500 = hug_realloc(ptr500, 200);
	print_heap();
	ptr150 = hug_realloc(ptr150, 100);
	print_heap();
	ptr500 = hug_realloc(ptr500, 0);
	print_heap();
	ptr250 = hug_malloc(250);
	print_heap();
	ptr250 = hug_realloc(ptr250, 1000);
	print_heap();
	hug_free(ptr300);
	hug_free(ptr250);
	hug_free(ptr150);
	assert_msg(offset == 0, "Free did not return all memory");
	
	printf("Finished malloctest.\n");
	
	free(heap);
	heap = NULL;
	heapsize = 0;
	offset = 0;
}
