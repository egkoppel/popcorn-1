#include "idt.hxx"

using namespace idt;

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
