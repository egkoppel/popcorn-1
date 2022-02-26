#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <serial.h>
#include <termcolor.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define TABSIZE 8

extern uint16_t x, y;
extern uint32_t col;
extern uint16_t termsize_x, termsize_y;

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
		
		if (x >= termsize_x) goto newline;
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
	static char itoabuf[72];
	
	while ((c = *fmt++)) {
		if (c == '%') {
			struct {
				bool left_justify; // NOT SUPPORTED
				bool force_sign; // NOT SUPPORTED
				bool space_if_no_sign; // NOT SUPPORTED
				bool num_mode; // NOT SUPPORTED
				bool zeropad;
				int width;
				int precision; // only supported for floats and strings
				enum printf_length_specifiers length;
			} options = {false};
			options.width = options.precision = -1;
			
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
						options.space_if_no_sign = true;
						break;
					
					case '#':
						options.num_mode = true;
						break;
					
					case '0':
						options.zeropad = true;
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
					if ('0' <= *fmt && *fmt <= '9') options.width = 0;
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
						if ('0' <= *fmt && *fmt <= '9') options.precision = 0;
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
			long len;
			char *str;
			//double f_val; // for floats, which currently are unsupported so are commented out
			//uint64_t f_intpart, f_floatpart;
			//bool neg;
			switch (*fmt) {
				case 'c': // todo: handle %lc
					kfputc(stream, va_arg(args, int /*char*/));
					break;
				
				case 'u':
				case 'o':
				case 'x':
				case 'X':
				case 'b': // unofficial extension: formats as binary
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
					else if (*fmt == 'b') base = 2;
					utoa(un, itoabuf, base);
					len = strlen(itoabuf);
					if (options.width != -1) {
						options.width -= len;
						while (options.width > 0) {
							kfputc(stream, options.zeropad ? '0' : ' ');
							--options.width;
						}
					}
					kfputs(stream, itoabuf);
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
					itoa(n, itoabuf, 10);
					if (n < 0) kfputc(stream, '-');
					len = strlen(itoabuf);
					if (options.width != -1) {
						options.width -= len;
						while (options.width > 0) {
							kfputc(stream, options.zeropad ? '0' : ' ');
							--options.width;
						}
					}
					kfputs(stream, n < 0 ? itoabuf + 1 : itoabuf);
					break;
				
				case 'p':
					p = va_arg(args, void*);
					if (p == NULL && !(options.length == PRINTF_LENGTH_l)) {
						kfputs(stream, "(nil)");
						break;
					}
					kfputs(stream, "0x");
					utoa((uint64_t)p, itoabuf, 16);
					for(size_t i = 0; i < sizeof(void*) * 2 - strlen(itoabuf); ++i) kfputc(stream, '0');
					kfputs(stream, itoabuf);
					break;
				
				case 'f':
				case 'F':
				/* // while we compile with -mno-sse, floats are basically broken, so this won't work anyway. If we ever enable floats again, we can uncomment this.
					if (options.precision < 0) options.precision = 6;
					
					//f_val = va_arg(args, double);
					f_val = ((union{uint64_t i; double f;})va_arg(args, uint64_t)).f;
					if ((neg = (f_val < 0.0))) {
						f_val *= -1.0;
					}
					f_intpart = (uint64_t)f_val;
					utoa(f_intpart, itoabuf, 10);
					const char too_big[] = "TOOBIG";
					if (f_val > (double)UINT64_MAX) memcpy(itoabuf, too_big, sizeof(too_big));
					
					len = (neg ? 1 : 0) + strlen(itoabuf) + 1 + options.precision;
					
					f_val -= f_intpart;
					for (int i = 0; i < options.precision; ++i) f_val *= 10;
					f_floatpart = (uint64_t)f_val;
					
					while(len < options.width) {
						kfputc(stream, options.zeropad ? '0' : ' ');
						--options.width;
					}
					if (neg) kfputc(stream, '-');
					kfputs(stream, itoabuf);
					kfputc(stream, '.');
					if (options.precision) {
						utoa(f_floatpart, itoabuf, 10);
						kfputs(stream, itoabuf);
						if ((long)strlen(itoabuf) != options.precision) {
							int diff = options.precision - (long)strlen(itoabuf);
							while (diff > 0) {
								kfputc(stream, '?');
								--diff;
							}
						}
					}
					break;
				*/
				case 'e': // todo: other floats
				case 'E':
				case 'g':
				case 'G':
				case 'a':
				case 'A':
					kfputs(stream, "[UNSUPPORTED]");
					break;
				
				case 's': // todo: handle %ls
					str = va_arg(args, char*);
					len = strlen(str);
					
					if (options.precision >= 0) len = MIN((long)len, options.precision);
					
					if (len < options.width) {
						for (int i = 0; i < options.width - len; ++i) kfputc(stream, ' ');
					}
					
					for (int i = 0; i < len; ++i) kfputc(stream, *str++);
					
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

void term_clear() {
	memset(FRAMEBUFFER, 0, framebuffer_pitch * psf_height * termsize_y); // zero whole screen
	x = 0;
	y = 0;
}
