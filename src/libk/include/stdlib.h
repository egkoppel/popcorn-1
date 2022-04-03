#ifndef _HUG_STDLIB_H
#define _HUG_STDLIB_H

#include <stdint.h>
#include <stddef.h>

#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0
#define MB_CUR_MAX 1
#ifndef NULL
#define NULL ((void*)0)
#endif
#define RAND_MAX 6

typedef int div_t;
typedef long ldiv_t;
typedef long long lldiv_t;

#ifdef __cplusplus
extern "C" {
#endif

double atof(const char* str);
int atoi(const char *s);
int atoi_p(const char **s);
long atol(const char *s);
long long atoll(const char *s);
double strtod(const char* str, char** endptr);
float strtof(const char* str, char** endptr);
long strtol(const char* str, char** endptr);
long double strtold(const char* str, char** endptr);
long long strtoll(const char* str, char** endptr);
unsigned long strtoul(const char* str, char** endptr);
unsigned long long strtoull(const char* str, char** endptr);
char* itoa(int64_t val, char *str, int base);
char* utoa(uint64_t val, char *str, int base);

int rand(void);
void srand(unsigned int seed);

void* calloc(size_t num, size_t size);
void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);

void abort(void);
int atexit(void (*func)(void));
int at_quick_exit(void (*func)(void));
void exit(int status);
char* getenv(const char* name);
void quick_exit(int status);
int system(const char* command);
void _Exit(int status);

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*,const void*));
void qsort(void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

int abs(int n);
div_t div(int numer, int denom);
int labs(long n);
ldiv_t ldiv(long numer, long denom);
int llabs(long long n);
lldiv_t lldiv(long long numer, long long denom);

int mblen(const char* pmb, size_t max);
int mbtowc(wchar_t* pwc, const char* pmb, size_t max);
int wctomb(char* pmb, wchar_t wc);

size_t mbstowcs(wchar_t* dest, const char* src, size_t max);
size_t wcstombs(char* dest, const wchar_t* src, size_t max);

#ifdef __cplusplus
}
#endif

#endif
