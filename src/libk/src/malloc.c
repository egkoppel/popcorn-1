#include <stdbool.h>
#include <stdint.h>

#include <memory.h>
#include <string.h>
#include <assert.h>
#include <utils.h>

#include <malloc.h>

#define HEADER_PADDING 0xCAFEBABE
#define FOOTER_PADDING 0xDEADBEEF12345678L

const int PAGE_SIZE = 4096;
const int MALLOC_ALIGNMENT = 16;

STATIC_ASSERT(sizeof(Header) % MALLOC_ALIGNMENT == 0)
STATIC_ASSERT(sizeof(Footer) % MALLOC_ALIGNMENT == 0)

static Header *first_free = NULL;
static void *heap_start = NULL;
static void *heap_end = NULL;
static bool first_malloc = true;

void* malloc(size_t size) {
	if (first_free != NULL) {
		assert_msg(first_free->pad == HEADER_PADDING, "Corrupted header");
		assert(first_free->prev_free == NULL);
	}
	
	size = ALIGN_UP(size, MALLOC_ALIGNMENT);
	
	if (first_free == NULL) {
		size_t extend = ALIGN_UP(size + sizeof(Header) + sizeof(Footer), PAGE_SIZE);
		first_free = (Header*)sbrk(extend);
		
		if (first_malloc) {
			heap_end = heap_start = (void*)first_free;
			first_malloc = false;
		}
		heap_end = (void*)((char*)heap_end + extend);
		
		*first_free = (Header){
			extend - sizeof(Header) - sizeof(Footer),
			NULL, NULL,
			true,
			HEADER_PADDING
		};
		Footer *new_footer = (Footer*)((char*)first_free + extend - sizeof(Footer));
		*new_footer = (Footer){first_free, FOOTER_PADDING};
	}
	
	Header *free_space = first_free, *prev_free_space = NULL;
	while (free_space->next_free != NULL && free_space->size < size) {
		assert_msg(free_space->pad == HEADER_PADDING, "Corrupted header in list of free spaces");
		assert_msg(free_space->size <= (intptr_t)heap_end - (intptr_t)heap_start - sizeof(Header) - sizeof(Footer), "Free space is bigger than the heap (impossible)");
		assert_msg(free_space->is_free, "Element of list of free spaces is not free");
		
		prev_free_space = free_space;
		free_space = free_space->next_free;
	}
	
	assert_msg(free_space->pad == HEADER_PADDING, "Corrupted header in list of free spaces");
	assert_msg(free_space->size <= (intptr_t)heap_end - (intptr_t)heap_start - sizeof(Header) - sizeof(Footer), "Free space is bigger than the heap (impossible)");
	assert_msg(free_space->is_free, "Element of list of free spaces is not free");
	
	if (free_space->size < size) { // failed to find any free space big enough
		assert(free_space->next_free == NULL);
		
		Footer *end_footer = (Footer*)((char*)heap_end - sizeof(Footer));
		Header *end = end_footer->header;
		
		assert_msg(end_footer->pad == FOOTER_PADDING, "Corrupted footer");
		assert_msg(end->pad == HEADER_PADDING, "Corrupted header");
		
		size_t extend;
		if (end->is_free) {
			extend = ALIGN_UP(size - end->size, PAGE_SIZE);
		} else {
			extend = ALIGN_UP(size + sizeof(Header) + sizeof(Footer), PAGE_SIZE);
		}
		
		Header *new_free_space = sbrk(extend);
		heap_end = (void*)((char*)heap_end + extend);
		Footer *new_footer = (Footer*)((char*)new_free_space + extend - sizeof(Footer));
		
		if (end->is_free) {
			free_space = end;
			
			*new_footer = (Footer){free_space, FOOTER_PADDING};
			free_space->size += extend;
			
			prev_free_space = free_space->prev_free;
			if (prev_free_space) {
				assert(prev_free_space->is_free);
				assert(prev_free_space->pad == HEADER_PADDING);
			}
		} else {
			*new_free_space = (Header){
				extend - sizeof(Header) - sizeof(Footer),
				free_space, NULL,
				true,
				HEADER_PADDING
			};
			*new_footer = (Footer){new_free_space, FOOTER_PADDING};
			
			free_space->next_free = new_free_space;
			
			prev_free_space = free_space;
			free_space = free_space->next_free;
			
			assert(free_space->next_free == NULL);
		}
	}
	
	assert(free_space->is_free);
	assert(free_space->size >= size);
	
	bool perfect_size = ((free_space->size < sizeof(Header) + sizeof(Footer)) ||
						 (size > free_space->size - sizeof(Header) - sizeof(Footer))); // "perfect size" if we couldn't fit it in if we split it by creating a new header and footer
	
	if (perfect_size) {
		free_space->is_free = false;
		
		if (free_space->prev_free != NULL) free_space->prev_free->next_free = free_space->next_free;
		if (free_space->next_free != NULL) free_space->next_free->prev_free = free_space->prev_free;
		
		if (free_space == first_free) first_free = first_free->next_free;
		
		free_space->prev_free = free_space->next_free = NULL; // just good policy to zero these out
		
		if (first_free != NULL) assert(first_free->prev_free == NULL);
		return (void*)((char*)free_space + sizeof(Header));
	} else {
		Footer *new_footer = (Footer*)((char*)free_space + sizeof(Header) + size);
		Header *new_header = (Header*)((char*)new_footer + sizeof(Footer));
		Footer *existing_footer = (Footer*)((char*)free_space + sizeof(Header) + free_space->size);
		
		assert_msg(existing_footer->pad == FOOTER_PADDING, "Corrupted footer");
		existing_footer->header = new_header;
		
		*new_header = (Header){
			(intptr_t)existing_footer - (intptr_t)new_header - sizeof(Header),
			prev_free_space, free_space->next_free,
			true,
			HEADER_PADDING
		};
		if (free_space->next_free != NULL) free_space->next_free->prev_free = new_header;
		if (prev_free_space != NULL) prev_free_space->next_free = new_header;
		else first_free = new_header; // if prev_free_space == NULL, free_space == first_free and so first free should now be the moved/new header
		
		*new_footer = (Footer){free_space, FOOTER_PADDING};
		
		free_space->is_free = false;
		free_space->size = size;
		free_space->next_free = free_space->prev_free = NULL;
		
		if (first_free != NULL) assert(first_free->prev_free == NULL);
		return (void*)((char*)free_space + sizeof(Header));
	}
}

void *calloc(const size_t num, const size_t size) {
	void *ptr = malloc(size * num);
	memset(ptr, 0, size * num);
	return ptr;
}

void free(void *ptr) {
	if (ptr == NULL) return;
	
	if (first_free != NULL) {
		assert_msg(first_free->pad == HEADER_PADDING, "First element of free spaces has corrupted header");
		assert(first_free->prev_free == NULL);
	}
	
	Header *header = (Header*)((char*)ptr - sizeof(Header));
	
	assert_msg(header->pad == HEADER_PADDING, "Space passed to free has corrupted header");
	assert_msg(!header->is_free, "Space passed to free is not free (Double free?)");
	assert_msg(header->size <= (intptr_t)heap_end - (intptr_t)heap_start - sizeof(Header) - sizeof(Footer), "Space passed to free is bigger than heap (impossible)");
	header->is_free = true;
	
	Footer *const prev_footer = ((Footer*)((char*)header - sizeof(Footer)));
	Header *prev_header = NULL;
	assert(heap_start != NULL);
	if ((void*)prev_footer >= heap_start) {
		prev_header = prev_footer->header;
	}
	Header *const next_header = (Header*)((char*)ptr + header->size + sizeof(Footer));
	
	bool in_linked_list = false;
	if ((void*)prev_header >= heap_start && prev_header->is_free) {
		assert_msg(prev_footer->pad == FOOTER_PADDING, "Preceding block has corrupted footer");
		assert_msg(prev_header->pad == HEADER_PADDING, "Preceding block has corrupted header");
		
		prev_header->size += header->size + sizeof(Header) + sizeof(Footer);
		Footer *footer = (Footer*)((char*)header + sizeof(Header) + header->size);
		footer->header = prev_header;
		header = prev_header;
		in_linked_list = true;
		
		assert_msg(prev_header->size <= (intptr_t)heap_end - (intptr_t)heap_start - sizeof(Header) - sizeof(Footer), "Preceding block is bigger than heap (impossible)");
	}
	
	if ((void*)next_header < heap_end && next_header->is_free) {
		assert_msg(next_header->pad == HEADER_PADDING, "Next block has corrupted header");
		
		// remove from linked list
		if (next_header->next_free != NULL) next_header->next_free->prev_free = next_header->prev_free;
		if (next_header->prev_free != NULL) next_header->prev_free->next_free = next_header->next_free;
		
		// expand header
		header->size += next_header->size + sizeof(Header) + sizeof(Footer);
		Footer *next_footer = (Footer*)((char*)next_header + sizeof(Header) + next_header->size);
		assert_msg(next_footer->pad == FOOTER_PADDING, "Next block has corrupted footer");
		next_footer->header = header;
		
		if (next_header == first_free) {
			// `header` already in linked list, but we have just removed first_free, so correct it
			first_free = first_free->next_free; // may set first_free = NULL if heap is like [freeing free] since we remove the only element in list of free spaces
			if (first_free != NULL) first_free->prev_free = NULL;
		}
		
		assert_msg(next_header->size <= (intptr_t)heap_end - (intptr_t)heap_start - sizeof(Header) - sizeof(Footer), "Next block is bigger than heap (impossible)");
	}
	
	if (!in_linked_list) {
		// prepend free space to linked list of free spaces
		if (first_free != NULL) assert(first_free->prev_free == NULL);
		
		header->prev_free = NULL;
		
		if (first_free != NULL) first_free->prev_free = header;
		header->next_free = first_free;
		
		first_free = header;
	}
	
	assert_msg(first_free->pad == HEADER_PADDING, "Post-free, the first free space has a corrupted header");
	assert_msg(first_free != NULL, "Freed space was not added to linked list of free spaces");
	assert(first_free->prev_free == NULL);
}

Header* __hug_malloc_get_first_free() {
	return first_free;
}

void __hug_malloc_clear_first_free() {
	first_free = NULL;
}

void __hug_malloc_set_first_malloc() {
	first_malloc = true;
}
