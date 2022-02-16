#ifndef _HUGOS_UTILS_H
#define _HUGOS_UTILS_H

#define ALIGN_UP(ptr, alignment) ((typeof(ptr))(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1)))
#define ADD_BYTES(ptr, offset) ((typeof(ptr))((uintptr_t)ptr + offset))

#endif
