#include "idt.hxx"

using namespace idt;

struct __attribute__((packed)) idt_ptr {
	uint16_t size;
	uint64_t address;
};

void IDT::load() {
	idt_ptr ptr;
	ptr.size = sizeof(IDT) - 1;
	ptr.address = reinterpret_cast<uint64_t>(this);

	__asm__ volatile("lidt %0" : : "m"(ptr));
}

entry::entry() {
	this->pointer_low = 0;
	this->segment_selector = 0;
	this->ist = 0;
	this->type = 0;
	this->_1 = 0;
	this->dpl = 0;
	this->present = 0;
	this->pointer_middle = 0;
	this->pointer_high = 0;
	this->_2 = 0;
}

IDT::IDT() {
	for (int i = 0; i < 15; i++) {
		this->entries[i] = entry();
	}
}

void IDT::add_entry(uint8_t index, uint8_t dpl, void(*handler)(), uint8_t ist) {
	uint64_t ptr = (uint64_t)handler;

	this->entries[index].pointer_low = (uint16_t)ptr;
	this->entries[index].segment_selector = 0x8;
	this->entries[index].ist = ist;
	this->entries[index].type = 0xE;
	this->entries[index]._1 = 0;
	this->entries[index].dpl = dpl;
	this->entries[index].present = 1;
	this->entries[index].pointer_middle = (uint16_t)(ptr >> 16);
	this->entries[index].pointer_high = (uint32_t)(ptr >> 32);
	this->entries[index]._2 = 0;
}
