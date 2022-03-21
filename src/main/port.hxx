#ifndef _HUGOS_PORT_H
#define _HUGOS_PORT_H

#include <stdint.h>

void io_wait(void);

template<typename T> class Port {
	public:
	Port() = delete;
};

template<> class Port<uint8_t> {
	public:
	Port(uint16_t port) : port(port) {}
	void write(uint8_t val) {
		__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(this->port));
	}
	uint8_t read() {
		uint8_t ret;
		__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};

template<> class Port<uint16_t> {
	public:
	Port(uint16_t port) : port(port) {}
	void write(uint16_t val) {
		__asm__ volatile ("out %0, %1" : : "a"(val), "Nd"(this->port));
	}
	uint16_t read() {
		uint16_t ret;
		__asm__ volatile ("in %1, %0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};

template<> class Port<uint32_t> {
	public:
	Port(uint16_t port) : port(port) {}
	void write(uint32_t val) {
		__asm__ volatile ("out %0, %1" : : "a"(val), "Nd"(this->port));
	}
	uint32_t read() {
		uint32_t ret;
		__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};

#endif