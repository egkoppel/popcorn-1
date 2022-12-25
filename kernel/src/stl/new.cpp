#include <new>
#include <stdlib.h>

void *operator new(size_t size) {
	void *r = malloc(size);
	if (!r) throw std::bad_alloc();
	return r;
}

void *operator new[](size_t size) {
	void *r = malloc(size);
	if (!r) throw std::bad_alloc();
	return r;
}

void operator delete(void *p) {
	free(p);
}

void operator delete[](void *p) {
	free(p);
}
