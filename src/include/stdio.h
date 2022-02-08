#ifndef _HUG_STDIO_H
#define _HUG_STDIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void kputc(char c);
void kputs(const char* str);
void kvprintf(const char *fmt, va_list args);
void kprintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif