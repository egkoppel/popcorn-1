#ifndef _HUGOS_IDT_H
#define _HUGOS_IDT_H

#include <stdint.h>

struct __attribute__((packed)) idt_entry {
	uint16_t pointer_low;
	uint16_t segment_selector;
	uint8_t _0;
	uint8_t type : 4;
	uint8_t _1 : 1;
	uint8_t dpl : 2;
	uint8_t present : 1;
	uint16_t pointer_middle;
	uint32_t pointer_high;
	uint32_t _2;
};

struct __attribute__((packed)) IDT {
	idt_entry entries[16];

	IDT();
	void load();
	void add_entry(uint8_t index, uint8_t dpl, void(*handler)());
};

#endif

