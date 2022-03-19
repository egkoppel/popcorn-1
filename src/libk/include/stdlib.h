#ifndef _HUG_STDLIB_H
#define _HUG_STDLIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char* itoa(int64_t val, char *str, int base);
char* utoa(uint64_t val, char *str, int base);
int atoi_p(const char **s);
int atoi(const char *s);

#ifdef __cplusplus
}
#endif

#endif
