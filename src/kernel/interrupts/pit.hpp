#ifndef _HUGOS_PIT_H
#define _HUGOS_PIT_H

#include "../main/port.h"
#include <bitset>

namespace pit {
	class Pit {
		private:
		using command_t = struct {
			uint8_t bcd: 1;
			uint8_t mode: 3;
			uint8_t access: 2;
			uint8_t channel: 2;
		};

		using access_t = enum: uint8_t {
			LATCH_COUNT = 0,
			LOBYTE_ONLY = 1,
			HIBYTE_ONLY = 2,
			LOBYTE_HIBYTE = 3
		};

		void update();

		public:
		using mode_t = enum: uint8_t {
			ONESHOT = 1,
			RATE_GENERATOR = 2,
			SQUARE_WAVE = 3
		};

		private:
		Port<uint8_t> channel0_data = Port<uint8_t>(0x40);
		Port<uint8_t> channel1_data = Port<uint8_t>(0x41);
		Port<uint8_t> channel2_data = Port<uint8_t>(0x42);
		Port<uint8_t> command = Port<uint8_t>(0x43);

		uint16_t divisor = 0;
		mode_t mode = SQUARE_WAVE;

		public:
		void set_divisor(uint16_t divisor);
		void set_mode(mode_t mode);
		void set_frequency(uint16_t frequency);
	};
}

extern pit::Pit timer;

#endif