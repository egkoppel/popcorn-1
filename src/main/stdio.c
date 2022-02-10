#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include <vga.h>

#define CONSOLE_SIZE_X 80
#define CONSOLE_SIZE_Y 25
#define TABSIZE 8

extern uint8_t x, y, col;

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
		if (x >= CONSOLE_SIZE_X) goto newline;
	} else if (c == '\n') {
		newline:
		x = 0;
		y++;
		if (y >= CONSOLE_SIZE_Y) {
			y = CONSOLE_SIZE_Y - 1;
			shift_up();
		}
	} else {
		*(uint16_t*)((void*)0xB8000 + 2*(x + CONSOLE_SIZE_X*y)) = vga_entry(c, col);
		x++;
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

int kvprintf(const char *fmt, va_list args) {
	int len, ret = 0;
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
					kputc(va_arg(args, int /*char*/));
					break;
				
				case 'u':
				case 'o':
				case 'x':
				case 'X':
					if (options.prepend == PRINTF_PREPEND_NUM_MODE) {
						if (*fmt == 'o') kputc('0');
						else if (*fmt != 'd') kputs("0x"); // 'x' or 'X'
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
					kputs(itoa(un, itoabuf, base));
					ret += strlen(itoabuf);
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
					kputs(itoa(n, itoabuf, 10));
					ret += strlen(itoabuf);
					break;
				
				case 'p':
					p = va_arg(args, void*);
					if (p == NULL) {
						kputs("(nil)");
						break;
					}
					kputs("0x");
					itoa((uint64_t)p, itoabuf, 16);
					len = strlen(itoabuf);
					ret += len;
					for(int i = 0; i < sizeof(void*) * 2 - len; ++i) kputc('0');
					kputs(itoabuf);
					break;
				
				case 'f': // todo: floats
				case 'F':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
				case 'a':
				case 'A':
					kputs("[FLOAT]");
					break;
				
				case 's': // todo: handle %ls
					kputs(va_arg(args, char*));
					break;
				
				case 'n':
					switch (options.length) {
						case PRINTF_LENGTH_hh: *(va_arg(args, char*)) = ret; break;
						case PRINTF_LENGTH_h:  *(va_arg(args, short int*)) = ret; break;
						case PRINTF_LENGTH_l:  *(va_arg(args, long int*)) = ret; break;
						case PRINTF_LENGTH_ll: *(va_arg(args, long long int*)) = ret; break;
						case PRINTF_LENGTH_j:  *(va_arg(args, intmax_t*)) = ret; break;
						case PRINTF_LENGTH_z:  *(va_arg(args, size_t*)) = ret; break;
						case PRINTF_LENGTH_t:  *(va_arg(args, ptrdiff_t*)) = ret; break;
						default:               *(va_arg(args, int*)) = ret; break;
					}
					break;
					
				case '%':
					kputc('%');
					break;
					
				default:
					break;
			}
			++fmt;
		} else {
			kputc(c);
			++ret;
		}
	}
	return ret;
}

int kprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int ret = kvprintf(fmt, args);

	va_end(args);
	
	return ret;
}

void handle_esc_code(int code) {
	switch (code) {
		case 0: col = 0x0F; break;
		case 30: set_fg_col(VGA_COLOR_BLACK); break;
		case 31: set_fg_col(VGA_COLOR_RED); break;
		case 32: set_fg_col(VGA_COLOR_GREEN); break;
		case 33: set_fg_col(VGA_COLOR_YELLOW); break;
		case 34: set_fg_col(VGA_COLOR_BLUE); break;
		case 35: set_fg_col(VGA_COLOR_MAGENTA); break;
		case 36: set_fg_col(VGA_COLOR_CYAN); break;
		case 37: set_fg_col(VGA_COLOR_WHITE); break;
	}
}