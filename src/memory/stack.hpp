#ifndef _HUGOS_STACK_H
#define _HUGOS_STACK_H

#include <stdint.h>

extern "C" struct Stack {
	public:
	uint64_t top;
	uint64_t bottom;

	Stack(uint64_t size);
	Stack(uint64_t top, uint64_t bottom): top(top), bottom(bottom) {}
	~Stack();
};

#endif