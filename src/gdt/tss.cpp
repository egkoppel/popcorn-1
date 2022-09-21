#include "tss.hpp"

using namespace tss;

TSS::TSS() {
	this->_res0 = 0;
	this->_res1 = 0;
	this->_res2 = 0;
	this->_res3 = 0;
	for (auto& i : this->interrupt_stack_table) {
		i = 0;
	}
	for (auto& i : this->privilege_stack_table) {
		i = 0;
	}
}

void TSS::load(uint16_t gdt_index) {
	asm volatile("ltr %w0" : : "q" (gdt_index*8));
}
