#include "port.h"

void io_wait(void) {
	__asm__ volatile ("outb %b0, $0x80" : : "a"(0));
}
