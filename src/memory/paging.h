#ifndef _HUGOS_PAGING_H
#define _HUGOS_PAGING_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint64_t present: 1;
	uint64_t writable: 1;
	uint64_t user_accessible: 1;
	uint64_t write_through: 1;
	uint64_t cache_disabled: 1;
	uint64_t accessed: 1;
	uint64_t dirty: 1;
	uint64_t huge: 1;
	uint64_t global: 1;
	uint64_t available_0: 3;
	uint64_t address : 40;
	uint64_t available_1 : 11;
	uint64_t no_execute : 1;
} page_table_entry;

#endif
