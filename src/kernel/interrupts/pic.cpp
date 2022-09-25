#include "pic.hpp"
#include "../main/port.h"

using namespace pic;

namespace commands { enum commands {
	INIT = 0x11,
	EOI = 0x20,
	READ_IRR = 0x0B,
	READ_ISR = 0x0A,
	MODE_8086 = 0x01
};}

void Pic::EOI() {
	this->send_command(commands::EOI);
}

uint8_t Pic::read_mask() {
	return this->data.read();
}

void Pic::write_data(uint8_t mask) {
	this->data.write(mask);
}

void Pic::send_command(uint8_t command) {
	this->command.write(command);
}

void ATChainedPIC::acknowledge_irq(uint8_t irq_line) {
	if (irq_line >= this->offset1 && irq_line < this->offset1 + 8) {
		this->master.EOI();
	} else if (irq_line >= this->offset2 && irq_line < this->offset2 + 8) {
		this->master.EOI();
		this->slave.EOI();
	}
}
