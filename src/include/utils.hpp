#ifndef _HUGOS_UTILS_H
#define _HUGOS_UTILS_H

#include <stdint.h>

template<class T> T* align_up(T* ptr, uintptr_t alignment) {
	return reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
}

template<class T> T* add_bytes(T* ptr, uintptr_t offset) {
	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + offset);
}

#endif
