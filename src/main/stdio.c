#include <stdint.h>
#include <stdarg.h>

extern uint8_t x;
extern uint8_t y;

uint16_t vga_entry(unsigned char uc, uint8_t color);
void shift_up();

void kputc(char c) {
	if (c == '\n') {
		x = 0;
		y++;
		if (y > 24) {
			y = 24;
			shift_up();
		}
	} else {
		*(uint16_t*)(0xB8000 + 2*(x + 80*y)) = vga_entry(c, 0x0F);
		x++;
	}
}

void kputs(const char* str) {
	char c;
	while ((c = *str++)) {
		kputc(c);
	}
}

void kvprintf(const char *fmt, va_list args) {
	char c;
	while ((c = *fmt++)) {
		if (c == '%') {
			
		} else kputc(c);
	}
}

void kprintf(const char *fmt, ...) {
	/*va_list args;
	va_start(args, fmt);

	vprintf(fmt, args);

	va_end(args);*/
	char c;
	while ((c = *fmt++)) {
		if (c == '%') {
			
		} else kputc(c);
	}
}