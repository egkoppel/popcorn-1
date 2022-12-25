#ifndef _HUG_MALLOC_TEST_SBRK_H
#define _HUG_MALLOC_TEST_SBRK_H

#include <stddef.h>
#include <stdint.h>

extern char *heap;
extern intptr_t offset;
extern size_t heapsize;

void *sbrk(intptr_t increment);

#endif
