#ifndef _HUG_STDIO_H
#define _HUG_STDIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void kputc(unsigned char c);
void kputs(const char* str);
int kvprintf(const char *fmt, va_list args);
int kprintf(const char *fmt, ...);
void handle_esc_code(int code);

#ifdef __cplusplus
}
#endif

#endif
