#ifndef _HUG_STRING_H
#define _HUG_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

	void *memcpy(void *dest, const void *src, size_t num);
	void *memmove(void *dest, const void *src, size_t num);
	char *strcpy(char *destination, const char *source);
	char *strncpy(char *destination, const char *source, size_t num);

	char *strcat(char *destination, const char *source);
	char *strncat(char *destination, const char *source, size_t num);

	int memcmp(const void *ptr1, const void *ptr2, size_t count);
	int strcmp(const char *str1, const char *str2);
	int strncmp(const char *s1, const char *s2, size_t n);
	int strcoll(const char *str1, const char *str2);
	int strncmp(const char *str1, const char *str2, size_t num);
	size_t strxfrm(char *destination, const char *source, size_t num);

#if 1
	void *memchr(const void *ptr, int value, size_t num);
	char *strchr(const char *str, int character);
	char *strpbrk(const char *str1, const char *str2);
	char *strrchr(const char *str, int character);
	char *strstr(const char *str1, const char *str2);
#endif

	//void* memchr(void *ptr, int value, size_t num);
	//char* strchr(char *str, int character);
	size_t strcspn(const char *str1, const char *str2);
	//char* strpbrk(char *str1, const char *str2);
	//char* strrchr(char *str, int character);
	size_t strspn(const char *str1, const char *str2);
	//char *strstr(char *str1, char *str2);
	char *strtok(char *str, const char *delimiters);

	void *memset(void *ptr, int value, size_t n);
	char *strerror(int errnum);
	size_t strlen(const char *str);

	char *strdup(const char *str1);

#ifdef __cplusplus
}
#endif

#endif
