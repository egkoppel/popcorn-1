#include <stdint.h>

template<class T> void align_up(T* &ptr, uintptr_t alignment) {
	ptr = reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) & ~(alignment - 1));
}

template<class T> void add_bytes(T* &ptr, uintptr_t offset) {
	ptr = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + offset);
}