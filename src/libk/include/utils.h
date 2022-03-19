#ifndef _HUGOS_UTILS_H
#define _HUGOS_UTILS_H

#include <panic.h>

#ifdef __cplusplus
	#define ALIGN_UP(ptr, alignment) ((decltype(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
	#define ALIGN_DOWN(ptr, alignment) ((decltype(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
	#define ADD_BYTES(ptr, offset) ((decltype(ptr))((uintptr_t)ptr + offset))

	#define IDIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

	#define UNWRAP(s) ({if (!s.valid) panic("Attempted to unwrap empty optional " #s); s.value;})
#else
	#define ALIGN_UP(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
	#define ALIGN_DOWN(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr) & ~(alignment - 1)))
	#define ADD_BYTES(ptr, offset) ((typeof(ptr))((uintptr_t)ptr + offset))

	#define IDIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

	#define UNWRAP(s) ({if (!s.valid) panic("Attempted to unwrap empty optional " #s); s.value;})
#endif

#endif
