#ifndef _HUGOS_PORT_H
#define _HUGOS_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" void io_wait(void);

template<typename T> class Port {
	public:
	Port() = delete;
};

template<> class Port<uint8_t> {
	public:
	Port(uint16_t port) : port(port) {}
	inline void write(uint8_t val) {
		__asm__ volatile ("outb %b0, %1" : : "a"(val), "Nd"(this->port));
	}
	inline uint8_t read() {
		uint8_t ret;
		__asm__ volatile ("inb %1, %b0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};

template<> class Port<uint16_t> {
	public:
	Port(uint16_t port) : port(port) {}
	inline void write(uint16_t val) {
		__asm__ volatile ("out %w0, %1" : : "a"(val), "Nd"(this->port));
	}
	inline uint16_t read() {
		uint16_t ret;
		__asm__ volatile ("in %1, %w0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};

template<> class Port<uint32_t> {
	public:
	Port(uint16_t port) : port(port) {}
	inline void write(uint32_t val) {
		__asm__ volatile ("out %d0, %1" : : "a"(val), "Nd"(this->port));
	}
	inline uint32_t read() {
		uint32_t ret;
		__asm__ volatile ("in %1, %d0" : "=a"(ret) : "Nd"(this->port));
		return ret;
	}
	private:
	uint16_t port;
};
#else
void io_wait(void);
#endif

#endif
