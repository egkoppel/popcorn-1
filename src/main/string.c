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

int memcmp(const void *ptr1, const void *ptr2, size_t count) {
	const char *s1 = ptr1;
	const char *s2 = ptr2;
	while (count-- > 0) {
	if (*s1++ != *s2++)
		return s1[-1] < s2[-1] ? -1 : 1;
	}
return 0;
}
