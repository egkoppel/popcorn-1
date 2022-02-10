#include <stdint.h>

#include <vga.h>

extern uint8_t col;

uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void shift_up() {
	for (int i = 0; i < 80*25; ++i) {
		uint16_t* p = (uint16_t*)0xB8000 + i;
		*p = *(p + 80);
	}
	for (int i = 0; i < 80; ++i) {
		uint16_t* p = (uint16_t*)0xB8000 + i + 80*24;
		*p = vga_entry(' ', 0x0F);
	}
}

void set_fg_col(uint8_t newcol) {
	uint8_t bg = col >> 4;
	col = newcol | bg << 4;
}
