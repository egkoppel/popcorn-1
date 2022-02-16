#include "idt.h"

typedef struct __attribute__((packed)) {
	uint16_t size;
	uint64_t address;
} idt_ptr;

idt_ptr ptr;

void idt_load(IDT* self) {
	sizeof(self->entries[0]);
	sizeof(idt_entry);
	ptr.size = sizeof(IDT) - 1;
	ptr.address = (uint64_t)self;

	__asm__ volatile("lidt %0" : : "m"(ptr));
}

void idt_init(IDT* self) {
	for (int i = 0; i < 15; i++) {
		self->entries[i].pointer_low = 0;
		self->entries[i].segment_selector = 0;
		self->entries[i]._0 = 0;
		self->entries[i].type = 0;
		self->entries[i]._1 = 0;
		self->entries[i].dpl = 0;
		self->entries[i].present = 0;
		self->entries[i].pointer_middle = 0;
		self->entries[i].pointer_high = 0;
		self->entries[i]._2 = 0;
	}
}

void idt_add_entry(IDT* self, uint8_t index, uint8_t dpl, void(*handler)()) {
	uint64_t ptr = (uint64_t)handler;

	self->entries[index].pointer_low = (uint16_t)ptr;
	self->entries[index].segment_selector = 0x8;
	self->entries[index]._0 = 0;
	self->entries[index].type = 0b1110;
	self->entries[index]._1 = 0;
	self->entries[index].dpl = 0;
	self->entries[index].present = 1;
	self->entries[index].pointer_middle = (uint16_t)(ptr >> 16);
	self->entries[index].pointer_high = (uint32_t)(ptr >> 32);
	self->entries[index]._2 = 0;
}