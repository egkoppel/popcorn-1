#include <stddef.h>

void* memcpy(void *dest, const void *src, size_t num) {
	for(size_t i = 0; i < num; ++i) {
		((char*)dest)[i] = ((char*)src)[i];
	}
	
	return dest;
}

void* memset(void *ptr, int value, size_t n) {
	for(size_t i = 0; i < n; ++i)
		((unsigned char*)ptr)[i] = (unsigned char)value;
	return ptr;
}

size_t strlen(const char *str) {
	size_t len = -1;
	while (str[++len] != '\0');
	return len;
}
