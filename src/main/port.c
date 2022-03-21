#include "port.h"
#include <stdint.h>

void outb(uint16_t port, uint8_t val) {
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
	/* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
	* Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
	* The  outb  %al, %dx  encoding is the only option for all other cases.
	* %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void outw(uint16_t port, uint16_t val) {
	__asm__ volatile ("out %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	__asm__ volatile ("in %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void outd(uint16_t port, uint32_t val) {
	__asm__ volatile ("out %0, %1" : : "a"(val), "Nd"(port));
}

uint32_t ind(uint16_t port) {
	uint32_t ret;
	__asm__ volatile ("in %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}
