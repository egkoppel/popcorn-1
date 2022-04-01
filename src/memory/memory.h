#ifndef _HUGOS_MEMORY_H
#define _HUGOS_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint64_t kernel_end;
	uint64_t current_break;
	bool initialised;
} sbrk_state_t;

extern sbrk_state_t global_sbrk_state;

void *sbrk(intptr_t increment); 

#ifdef __cplusplus
}
#endif

#endif
