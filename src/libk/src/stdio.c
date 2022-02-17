#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <serial.h>
#include <termcolor.h>

#define TABSIZE 8

extern uint16_t x, y;
extern uint32_t col;
extern uint32_t termsize_x, termsize_y;

extern char font_psf_start;
extern char font_psf_end;
extern char font_psf_size;

extern uint32_t psf_headersize;
extern uint32_t psf_numglyph;
extern uint32_t psf_bytesperglyph;
extern uint32_t psf_height;
extern uint32_t psf_width;

extern uint32_t framebuffer_pitch;
extern uint32_t framebuffer_width;
extern uint32_t framebuffer_height;
extern uint8_t framebuffer_bpp;

static size_t stdio_char_count_theoretical = 0;
static size_t stdio_char_count = 0;
static size_t stdio_char_limit = 0;
static char *stdio_strbuf; // for sprintf, snprintf

static FILE _stdin = { .fd = 0 };
static FILE _stdout = { .fd = 1 };
static FILE _stderr = { .fd = 2 };
static FILE _stdserial = { .fd = 3 };
static FILE _stdstrbuf = {.fd = 4 };

FILE *stdin = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;
FILE *stdserial = &_stdserial;
static FILE *stdstrbuf = &_stdstrbuf;

#define FRAMEBUFFER ((char*)0xffffff8040000000L)

void shift_up() {
	memmove(FRAMEBUFFER, FRAMEBUFFER + framebuffer_pitch * psf_height, framebuffer_pitch * psf_height * (termsize_y - 1)); // copy one line upwards
	memset(FRAMEBUFFER + framebuffer_pitch * psf_height * (termsize_y - 1), 0, framebuffer_pitch * psf_height); // zero the final line
}

void kputc(unsigned char c) {
	static bool esc = false;
	static int code = 0;
	
	if (esc) {
		switch (c) {
			case 'm':
				esc = false;
			case ';':
				handle_esc_code(code); // then fallthrough to reset code (necessary to parse next one, harmless if 'm')
			case '[':
				code = 0;
				break;
			default:
				code = 10*code + (c - '0');
				break;
		}
		return;
	}
	
	if (c == 0x1b) { // ESC
		esc = true;
	} else if (c == '\t') {
		do {
			x++;
		} while (x % TABSIZE);
		if (x >= termsize_x) goto newline;
	} else if (c == '\n') {
		newline:
		x = 0;
		y++;
		if (y >= termsize_y) {
			y = termsize_y - 1;
			shift_up();
		}
	} else {
		if (c >= psf_numglyph) return;
		
		char *fb_target = FRAMEBUFFER + y * framebuffer_pitch * psf_height + x * (psf_width + 1) * framebuffer_bpp / 8;
		for (uint32_t yy = 0; yy < psf_height; ++yy) {
			for (uint32_t xx = 0; xx < psf_width; ++xx) {
				uint32_t bit_addr = yy * psf_width + xx;
				char font_byte = *((char*)&font_psf_start + psf_headersize + c * psf_bytesperglyph + bit_addr / 8);
				bool bit = (font_byte << (bit_addr % 8)) & (1 << 7);
				*(uint32_t*)(fb_target + xx * framebuffer_bpp / 8) = bit ? col : 0;
			}
			fb_target += framebuffer_pitch;
		}
		
		x++;
	}
}

void kfputc(FILE* stream, unsigned char c) {
	++stdio_char_count_theoretical;
	
	if (stdio_char_count >= stdio_char_limit) return;
	++stdio_char_count;
	
	if (stream->fd == stdout->fd) {
		kputc(c);
	} else if (stream->fd == stderr->fd) {
		kputc(c);
	} else if (stream->fd == stdserial->fd) {
		write_serial(c);
	} else if (stream->fd == _stdstrbuf.fd) {
		*stdio_strbuf++ = c;
	} else {
		// TODO: real files
	}
}

inline void kfputs(FILE* stream, const char* str) {
	char c;
	while ((c = *str++)) {
		kfputc(stream, c);
	}
}

inline void kputs(const char* str) {
	char c;
	while ((c = *str++)) {
		kputc(c);
	}
}

enum printf_prepend_specifiers {
	PRINTF_PREPEND_NONE,
	
	PRINTF_PREPEND_NUM_MODE,
	PRINTF_PREPEND_SPACE,
	PRINTF_PREPEND_ZERO,
};

enum printf_length_specifiers {
	PRINTF_LENGTH_DEFAULT,
	
	PRINTF_LENGTH_hh,
	PRINTF_LENGTH_h,
	PRINTF_LENGTH_l,
	PRINTF_LENGTH_ll,
	PRINTF_LENGTH_j,
	PRINTF_LENGTH_z,
	PRINTF_LENGTH_t,
	PRINTF_LENGTH_L,
};

int _kvfprintf(FILE* stream, const char *fmt, va_list args) {
	char c;
	void *p;
	static char itoabuf[32];
	
	while ((c = *fmt++)) {
		if (c == '%') {
			struct {
				bool left_justify;
				bool force_sign;
				enum printf_prepend_specifiers prepend;
				int width;
				int precision;
				enum printf_length_specifiers length;
			} options = {false};
			
			bool done_with_flags = false;
			while (*fmt != '\0' && !done_with_flags) {
				switch (*fmt) { // flags
					case '-':
						options.left_justify = true;
						break;
						
					case '+':
						options.force_sign = true;
						break;
					
					case ' ':
						options.prepend = PRINTF_PREPEND_SPACE;
						break;
					
					case '#':
						options.prepend = PRINTF_PREPEND_NUM_MODE;
						break;
					
					case '0':
						options.prepend = PRINTF_PREPEND_ZERO;
						break;
					
					default:
						--fmt;
						done_with_flags = true;
				}
				++fmt;
			}
			
			switch (*fmt) { // width
				case '*':
					options.width = va_arg(args, int);
					break;
					
				default:
					while ('0' <= *fmt && *fmt <= '9') {
						options.width *= 10;
						options.width += *fmt - '0';
						++fmt;
					};
					break;
			}
			
			if (*fmt == '.') {
				++fmt;
				switch (*fmt) { // precision
					case '*':
						options.precision = va_arg(args, int);
						break;
						
					default:
						while ('0' <= *fmt && *fmt <= '9') {
							options.precision *= 10;
							options.precision += *fmt - '0';
							++fmt;
						};
						break;
				}
			}
			
			switch (*fmt) {
				case 'h':
					++fmt;
					if (*fmt == '\0') break;
					if (*fmt == 'h') options.length = PRINTF_LENGTH_hh;
					else {
						options.length = PRINTF_LENGTH_h;
						--fmt;
					}
					break;
					
				case 'l':
					++fmt;
					if (*fmt == '\0') break;
					if (*fmt == 'l') options.length = PRINTF_LENGTH_ll;
					else {
						options.length = PRINTF_LENGTH_l;
						--fmt;
					}
					break;
					
				case 'j':
					options.length = PRINTF_LENGTH_j;
					break;
					
				case 'z':
					options.length = PRINTF_LENGTH_z;
					break;
					
				case 't':
					options.length = PRINTF_LENGTH_t;
					break;
					
				case 'L':
					options.length = PRINTF_LENGTH_L;
					break;
				
				default:
					--fmt;
					break;
			}
			++fmt;
			
			int64_t n;
			uint64_t un;
			switch (*fmt) {
				case 'c': // todo: handle %lc
					kfputc(stream, va_arg(args, int /*char*/));
					break;
				
				case 'u':
				case 'o':
				case 'x':
				case 'X':
					if (options.prepend == PRINTF_PREPEND_NUM_MODE) {
						if (*fmt == 'o') kfputc(stream, '0');
						else if (*fmt != 'd') kfputs(stream, "0x"); // 'x' or 'X'
					}
					switch (options.length) {
						case PRINTF_LENGTH_hh: un = va_arg(args, int /*unsigned char*/); break;
						case PRINTF_LENGTH_h:  un = va_arg(args, int /*unsigned short int*/); break;
						case PRINTF_LENGTH_l:  un = va_arg(args, unsigned long int); break;
						case PRINTF_LENGTH_ll: un = va_arg(args, unsigned long long int); break;
						case PRINTF_LENGTH_j:  un = va_arg(args, uintmax_t); break;
						case PRINTF_LENGTH_z:  un = va_arg(args, size_t); break;
						case PRINTF_LENGTH_t:  un = va_arg(args, ptrdiff_t); break;
						default:               un = va_arg(args, unsigned int); break;
					}
					int base = 16;
					if (*fmt == 'o') base = 8;
					else if (*fmt == 'u') base = 10;
					kfputs(stream, utoa(un, itoabuf, base));
					break;
					
				case 'd':
				case 'i':
					switch (options.length) {
						case PRINTF_LENGTH_hh: n = va_arg(args, int /*char*/); break;
						case PRINTF_LENGTH_h:  n = va_arg(args, int /*short int*/); break;
						case PRINTF_LENGTH_l:  n = va_arg(args, long int); break;
						case PRINTF_LENGTH_ll: n = va_arg(args, long long int); break;
						case PRINTF_LENGTH_j:  n = va_arg(args, intmax_t); break;
						case PRINTF_LENGTH_z:  n = va_arg(args, size_t); break;
						case PRINTF_LENGTH_t:  n = va_arg(args, ptrdiff_t); break;
						default:               n = va_arg(args, int); break;
					}
					kfputs(stream, itoa(n, itoabuf, 10));
					break;
				
				case 'p':
					p = va_arg(args, void*);
					if (p == NULL) {
						kfputs(stream, "(nil)");
						break;
					}
					kfputs(stream, "0x");
					utoa((uint64_t)p, itoabuf, 16);
					for(size_t i = 0; i < sizeof(void*) * 2 - strlen(itoabuf); ++i) kfputc(stream, '0');
					kfputs(stream, itoabuf);
					break;
				
				case 'f': // todo: floats
				case 'F':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
				case 'a':
				case 'A':
					kfputs(stream, "[FLOAT]");
					break;
				
				case 's': // todo: handle %ls
					kfputs(stream, va_arg(args, char*));
					break;
				
				case 'n':
					switch (options.length) {
						case PRINTF_LENGTH_hh: *(va_arg(args, char*)) = stdio_char_count; break;
						case PRINTF_LENGTH_h:  *(va_arg(args, short int*)) = stdio_char_count; break;
						case PRINTF_LENGTH_l:  *(va_arg(args, long int*)) = stdio_char_count; break;
						case PRINTF_LENGTH_ll: *(va_arg(args, long long int*)) = stdio_char_count; break;
						case PRINTF_LENGTH_j:  *(va_arg(args, intmax_t*)) = stdio_char_count; break;
						case PRINTF_LENGTH_z:  *(va_arg(args, size_t*)) = stdio_char_count; break;
						case PRINTF_LENGTH_t:  *(va_arg(args, ptrdiff_t*)) = stdio_char_count; break;
						default:               *(va_arg(args, int*)) = stdio_char_count; break;
					}
					break;
					
				case '%':
					kfputc(stream, '%');
					break;
					
				default:
					break;
			}
			++fmt;
		} else {
			kfputc(stream, c);
		}
	}
	return stdio_char_count;
}

int kvfprintf(FILE* stream, const char *fmt, va_list args) {
	stdio_char_count = 0;
	stdio_char_limit = SIZE_MAX;
	
	return _kvfprintf(stream, fmt, args);
}

int kvprintf(const char *fmt, va_list args) {
	return kvfprintf(stdout, fmt, args);
}

int kfprintf(FILE *stream, const char *fmt, ...) {
	stdio_char_count = 0;
	stdio_char_limit = SIZE_MAX;
	
	va_list args;
	va_start(args, fmt);

	int ret = kvfprintf(stream, fmt, args);

	va_end(args);
	
	return ret;
}

int kprintf(const char *fmt, ...) {
	stdio_char_count = 0;
	stdio_char_limit = SIZE_MAX;
	
	va_list args;
	va_start(args, fmt);

	int ret = kvfprintf(stdout, fmt, args);

	va_end(args);
	
	return ret;
}

int kvsnprintf(char *str, size_t size, const char *fmt, va_list args) {
	stdio_strbuf = str;
	stdio_char_count = 0;
	stdio_char_count_theoretical = 0;
	stdio_char_limit = size;
	
	_kvfprintf(stdstrbuf, fmt, args);
	
	if (stdio_char_count >= stdio_char_limit) {
		*(--stdio_strbuf) = '\0'; // don't write off the end of the buffer
	} else {
		*stdio_strbuf = '\0';
	}
	
	return stdio_char_count_theoretical;
}

int ksnprintf(char *str, size_t size, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int ret = kvsnprintf(str, size, fmt, args);

	va_end(args);
	
	return ret;
}

int kvsprintf(char *str, const char *fmt, va_list args) {
	return kvsnprintf(str, SIZE_MAX, fmt, args);
}

int ksprintf(char *str, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int ret = kvsprintf(str, fmt, args);

	va_end(args);
	
	return ret;
}

void handle_esc_code(int code) {
	switch (code) {
		case 0:  col = COLOR_WHITE; break;
		case 30: col = COLOR_BLACK; break;
		case 31: col = COLOR_RED; break;
		case 32: col = COLOR_GREEN; break;
		case 33: col = COLOR_YELLOW; break;
		case 34: col = COLOR_BLUE; break;
		case 35: col = COLOR_MAGENTA; break;
		case 36: col = COLOR_CYAN; break;
		case 37: col = COLOR_WHITE; break;
	}
}
