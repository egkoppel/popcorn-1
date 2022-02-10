#ifndef _HUG_VGA_H
#define _HUG_VGA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_YELLOW = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_YELLOW = 14,
	VGA_COLOR_WHITE = 15,
};

uint16_t vga_entry(unsigned char uc, uint8_t color);
void shift_up();
void set_fg_col(uint8_t newcol);

#ifdef __cplusplus
}
#endif


#endif