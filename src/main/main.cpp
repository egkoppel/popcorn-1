#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

#include <termcolor.h>

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	kprintf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Mulitboot magic: 0x%x\n", multiboot_magic);
}
