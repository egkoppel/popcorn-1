#ifndef _HUG_MALLOC_H
#define _HUG_MALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct header {
	size_t size;
	struct header *prev_free, *next_free;
	bool is_free;
	uint32_t pad;
} Header;

typedef struct {
	Header *header;
	uint64_t pad;
} Footer;

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void free(void *ptr);

Header* __hug_malloc_get_first_free();
void __hug_malloc_clear_first_free();
void __hug_malloc_set_first_malloc();

#ifdef __cplusplus
}
#endif

#endif
