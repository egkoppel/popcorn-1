#include "idt.hxx"

using namespace idt;

struct __attribute__((packed)) idt_ptr {
	uint16_t size;
	uint64_t address;
};

void IDT::load() {
	idt_ptr ptr;
	ptr.size = sizeof(IDT) - 1;
	ptr.address = (uint64_t)this;

	__asm__ volatile("lidt %0" : : "m"(ptr));
}

IDT::IDT() {
	for (int i = 0; i < 15; i++) {
		this->entries[i].pointer_low = 0;
		this->entries[i].segment_selector = 0;
		this->entries[i].ist = 0;
		this->entries[i].type = 0;
		this->entries[i]._1 = 0;
		this->entries[i].dpl = 0;
		this->entries[i].present = 0;
		this->entries[i].pointer_middle = 0;
		this->entries[i].pointer_high = 0;
		this->entries[i]._2 = 0;
	}
}

void IDT::add_entry(uint8_t index, uint8_t dpl, void(*handler)()) {
	uint64_t ptr = (uint64_t)handler;

	this->entries[index].pointer_low = (uint16_t)ptr;
	this->entries[index].segment_selector = 0x8;
	this->entries[index].ist = 0;
	this->entries[index].type = 0xE;
	this->entries[index]._1 = 0;
	this->entries[index].dpl = dpl;
	this->entries[index].present = 1;
	this->entries[index].pointer_middle = (uint16_t)(ptr >> 16);
	this->entries[index].pointer_high = (uint32_t)(ptr >> 32);
	this->entries[index]._2 = 0;
}
