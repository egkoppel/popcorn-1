#ifndef _HUG_STDIO_H
#define _HUG_STDIO_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t fd;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
extern FILE* stdserial;

void kfputc(FILE *stream, unsigned char c);
void kputc(unsigned char c);

void term_clear();

void kfputs(FILE *stream, const char* str);
void kputs(const char* str);

int kvfprintf(FILE *stream, const char *fmt, va_list args);
int kvprintf(const char *fmt, va_list args);
int kfprintf(FILE *stream, const char *fmt, ...);
int kprintf(const char *fmt, ...);
int kvsnprintf(char *str, size_t size, const char *fmt, va_list args);
int ksnprintf(char *str, size_t size, const char *fmt, ...);
int kvsprintf(char *str, const char *fmt, va_list args);
int ksprintf(char *str, const char *fmt, ...);

void handle_esc_code(int code);

#ifdef __cplusplus
}
#endif

#endif
