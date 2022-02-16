#ifndef _HUG_STRING_H
#define _HUG_STRING_H

#include <stddef.h>

void* memcpy(void *dest, const void *src, size_t num);
void* memset(void *ptr, int value, size_t n);
size_t strlen(const char *str);
int memcmp(const void *ptr1, const void *ptr2, size_t count);

#endif
