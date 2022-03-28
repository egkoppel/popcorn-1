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
#include "malloctest3.h"

typedef struct dual {
	size_t size;
	unsigned char *libc, *hug;
} Dual;

enum operation {
	ALLOC,
	FREE,
	READ,
	WRITE,
	NUM_OPERATIONS
};

#define DUALS 1000
#define MAX_ALLOC_SIZE 5000
#define ITERS 10000

#define VERBOSE 0
#if VERBOSE
#define cprintf(...) printf(__VA_ARGS__)
#else
#define cprintf(...) do {} while(0)
#endif

void test_malloc3(unsigned int random_seed) {
	const size_t HEAPSIZE = DUALS * (MAX_ALLOC_SIZE + sizeof(Header) + sizeof(Footer));
	heap = malloc(HEAPSIZE);
	assert(heap != NULL);
	heapsize = HEAPSIZE;
	offset = 0;
	__hug_malloc_clear_first_free();
	__hug_malloc_set_first_malloc();
	
	srand(random_seed);
	cprintf("\nBeginning malloctest3.\n");
	
	Dual duals[DUALS];
	for (size_t i = 0; i < DUALS; ++i) duals[i] = (Dual){0, NULL, NULL};
	
	for (int i = 0; i < ITERS; ++i) {
		enum operation op = rand() % NUM_OPERATIONS;
		size_t ind = rand() % DUALS;
		
		if (op == ALLOC) {
			if (duals[ind].libc == NULL) { // not currently active
				size_t size = rand() % MAX_ALLOC_SIZE;
				duals[ind] = (Dual){
					size,
					malloc(size),
					hug_malloc(size)
				};
				assert(duals[ind].libc != NULL);
				assert(duals[ind].hug != NULL);
				
				cprintf("Allocated at duals[%lu]\n", ind);
				
				goto write;
			}
		} else if (op == FREE) {
			free(duals[ind].libc);
			hug_free(duals[ind].hug);
			duals[ind] = (Dual){0, NULL, NULL};
			cprintf("Freed duals[%lu]\n", ind);
		} else if (op == READ) {
			if (duals[ind].libc != NULL) {
				assert(memcmp(duals[ind].libc, duals[ind].hug, duals[ind].size) == 0);
				cprintf("Validated duals[%lu]\n", ind);
			}
		} else if (op == WRITE) {
			if (duals[ind].libc != NULL) {
				write:
				for (size_t j = 0; j < duals[ind].size; ++j) {
					unsigned char c = rand() % 255;
					duals[ind].libc[j] = c;
					duals[ind].hug[j] = c;
				}
			}
			cprintf("Written duals[%lu]\n", ind);
		} else {
			panic("invalid operation");
		}
	}
	
	for (size_t i = 0; i < DUALS; ++i) {
		free(duals[i].libc);
		hug_free(duals[i].hug);
	}
	
	assert_msg(offset == 0, "Free did not return all memory");
	
	cprintf("No corruption was found.\nFinished malloctest3.\n");
	
	free(heap);
	heap = NULL;
	heapsize = 0;
	offset = 0;
}
