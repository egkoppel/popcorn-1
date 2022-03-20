#include "tss.hxx"

using namespace tss;

TSS::TSS() {
	this->_res0 = 0;
	this->_res1 = 0;
	this->_res2 = 0;
	this->_res3 = 0;
	for (int i = 0; i < 7; i++) {
		this->interrupt_stack_table[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		this->privilege_stack_table[i] = 0;
	}
}

void TSS::load(uint16_t gdt_index) {
	asm volatile("ltr %w0" : : "q" (gdt_index*8));
}
