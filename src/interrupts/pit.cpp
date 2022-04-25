#include "pit.hpp"

using namespace pit;

pit::Pit timer = Pit();

void Pit::update() {
	auto c = Pit::command_t {
		.bcd = 0,
		.mode = this->mode,
		.access = access_t::LOBYTE_HIBYTE,
		.channel = 0
	};
	command.write(*reinterpret_cast<uint8_t*>(&c));
	channel0_data.write(this->divisor & 0xFF);
	channel0_data.write(this->divisor >> 8);
}

void Pit::set_divisor(uint16_t divisor) {
	this->divisor = divisor;
	this->update();
}

void Pit::set_mode(mode_t mode) {
	this->mode = mode;
	this->update();
}

void Pit::set_frequency(uint16_t frequency) {
	this->set_divisor(1193180 / frequency);
}
