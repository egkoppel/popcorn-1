#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

extern "C" inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

extern "C" void shift_up() {
	for (int i = 0; i < 80*25; ++i) {
		uint16_t* p = (uint16_t*)0xB8000 + i;
		*p = *(p + 80);
	}
	for (int i = 0; i < 80; ++i) {
		uint16_t* p = (uint16_t*)0xB8000 + i + 80*24;
		*p = vga_entry(' ', 0x0F);
	}
}

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	kprintf("Hello, world!\n");
	kprintf(itoa(multiboot_magic, 16));
}
