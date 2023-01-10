#ifndef _HUGOS_SERIAL_H
#define _HUGOS_SERIAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void write_serial1(char a);

#ifdef __cplusplus
}

	#include <arch/amd64/interrupts/port.hpp>

class SerialPort {
private:
	Port<uint8_t> data, interupt_enable, fifo_control, line_control, modem_contol, line_status, modem_status, scratch;

public:
	struct SerialPortError : public std::exception {
		const char *what() const noexcept override { return "Serial port borked"; }
	};

	explicit SerialPort(uint16_t port);

	int is_transmit_empty() const noexcept;
	void write(char a) noexcept;
	void print(char *s) noexcept;
};

extern std::optional<SerialPort> serial1;

#endif

#endif
