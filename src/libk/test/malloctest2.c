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
#include <string.h>

#include "sbrk.h"
#include "malloctest2.h"

typedef struct dual {
	size_t size;
	unsigned char *libc, *hug;
} Dual;

#define ALLOCS1 1000
#define ALLOCS2 1000
#define MAX_ALLOC_SIZE 5000
#define RANDOM_FREES 200
#define HEXDUMP_BYTES_PER_INTERVAL 4
#define HEXDUMP_BYTES_PER_LINE 48

#define VERBOSE 0
#if VERBOSE
#define cprintf(...) printf(__VA_ARGS__)
#else
#define cprintf(...) do {} while(0)
#endif

static void hexdump(const unsigned char *data, const unsigned char *cmp, const size_t len) {
	size_t i = 0;
	
	printf("\t");
	
	while (i < len) {
		if (cmp != NULL && data[i] != cmp[i]) {
			printf(TERMCOLOR_RED);
		}
		
		printf("%02x ", data[i]);
		
		if (cmp != NULL && data[i] != cmp[i]) {
			printf(TERMCOLOR_RESET);
		}
		
		++i;
		
		if (i % HEXDUMP_BYTES_PER_LINE == 0) {
			printf("\n\t");
		} else if (i % HEXDUMP_BYTES_PER_INTERVAL == 0) {
			printf("  ");
		}
	}
	
	if (len % HEXDUMP_BYTES_PER_LINE != 0) { // if len is a multiple of HEXDUMP_BYTES_PER_LINE we already printed a newline
		printf("\n");
	} else {
		printf("\b"); // backspace the tab char
	}
}

void test_malloc2(unsigned int random_seed) {
	const size_t HEAPSIZE = (ALLOCS1 + ALLOCS2) * (MAX_ALLOC_SIZE + sizeof(Header) + sizeof(Footer));
	heap = malloc(HEAPSIZE);
	assert(heap != NULL);
	heapsize = HEAPSIZE;
	offset = 0;
	__hug_malloc_clear_first_free();
	__hug_malloc_set_first_malloc();
	
	srand(random_seed);
	cprintf("\nBeginning malloctest2.\n");
	
	Dual duals[ALLOCS1 + ALLOCS2];
	
	size_t i;
	for (i = 0; i < ALLOCS1; ++i) {
		size_t size = rand() % MAX_ALLOC_SIZE;
		
		cprintf("About to allocate string %5lu of size %5lu...", i, size);
		fflush(stdout);
		
		duals[i].size = size;
		
		duals[i].libc = malloc(size);
		assert(duals[i].libc != NULL);
		duals[i].hug = hug_malloc(size);
		assert(duals[i].hug != NULL); // even though it should be literally impossible for this to happen
		
		for (size_t j = 0; j < size; ++j) {
			char c = rand() % 255; // fill with same data
			duals[i].hug[j] = c;
			duals[i].libc[j] = c;
		}
		cprintf(" \tAllocated at addr {libc = %18p, hug = %18p}\n", (void*)duals[i].libc, (void*)duals[i].hug);
	}
	cprintf("Allocated %i strings.\n", ALLOCS1);
	
	for (size_t j = 0; j < RANDOM_FREES; ++j) { // do some frees
		size_t ind = rand() % ALLOCS1;
		cprintf("Freeing string %5lu of size %5lu...", ind, duals[ind].size);
		fflush(stdout);
		
		if (duals[ind].libc == NULL || duals[ind].hug == NULL) {
			assert_msg(duals[ind].hug == NULL, "duals[%lu].libc == NULL  but  duals[%lu].hug != NULL", ind, ind);
			assert_msg(duals[ind].libc == NULL, "duals[%lu].hug == NULL  but  duals[%lu].libc != NULL", ind, ind);
			cprintf(" Already freed.\n");
			continue;
		}
		
		free(duals[ind].libc);
		duals[ind].libc = NULL;
		
		hug_free(duals[ind].hug);
		duals[ind].hug = NULL;
		
		cprintf(" Freed.\n");
	}
	cprintf("Freed %i random strings.\n", RANDOM_FREES);
	
	for (; i < ALLOCS1 + ALLOCS2; ++i) { // do some more allocs
		size_t size = rand() % MAX_ALLOC_SIZE;
		
		cprintf("About to allocate string %5lu of size %5lu...", i, size);
		fflush(stdout);
		
		duals[i].size = size;
		
		duals[i].libc = malloc(size);
		assert(duals[i].libc != NULL);
		duals[i].hug = hug_malloc(size);
		assert(duals[i].hug != NULL); // even though it should be literally impossible for this to happen
		
		for (size_t j = 0; j < size; ++j) {
			char c = rand() % 255; // fill with same data
			duals[i].hug[j] = c;
			duals[i].libc[j] = c;
		}
		cprintf(" \tAllocated at addr {libc = %18p, hug = %18p}\n", (void*)duals[i].libc, (void*)duals[i].hug);
	}
	cprintf("Allocated a further %i strings.\nNow checking for any corruption...\n", ALLOCS2);
	fflush(stdout);
	
	for (size_t j = 0; j < ALLOCS1 + ALLOCS2; ++j) {
		cprintf("Checking string %5lu of length %5lu... [libc = %18p, hug = %18p]", j, duals[j].size, (void*)duals[j].libc, (void*)duals[j].hug);
		fflush(stdout);
		
		if (duals[j].hug == NULL || duals[j].libc == NULL) { // freed
			assert_msg(duals[j].hug == NULL, "duals[%lu].libc == NULL  but  duals[%lu].hug != NULL", j, j);
			assert_msg(duals[j].libc == NULL, "duals[%lu].hug == NULL  but  duals[%lu].libc != NULL", j, j);
			cprintf(" Both have already been freed.\n");
			continue;
		}
		
		if (memcmp(duals[j].libc, duals[j].hug, duals[j].size) != 0) {
			cprintf("\n" TERMCOLOR_RED "CORRUPTION DETECTED, DUMPING CONTENTS" TERMCOLOR_RESET "\n");
			
			cprintf("duals[%lu].libc:\n", j);
			hexdump(duals[j].libc, NULL, duals[j].size);
			
			cprintf("duals[%lu].hug:\n", j);
			hexdump(duals[j].hug, duals[j].libc, duals[j].size);
		}
		assert_msg(memcmp(duals[j].libc, duals[j].hug, duals[j].size) == 0, "\tCorruption detected!");
		
		cprintf(" No corruption was detected.\n");
		
		free(duals[j].libc);
		hug_free(duals[j].hug);
	}
	
	assert_msg(offset == 0, "Free did not return all memory");
	
	cprintf("No corruption was found.\nFinished malloctest2.\n");
	
	free(heap);
	heap = NULL;
	heapsize = 0;
	offset = 0;
}
