#ifndef _HUGOS_PANIC_H
#define _HUGOS_PANIC_H

#include <termcolor.h>

#define panic(msg) \
	{ \
		kprintf("["TERMCOLOR_RED "ERR!" TERMCOLOR_RESET "]" TERMCOLOR_RED " Kernel panicked at %s:%d:\n\t%s\n", __FILE__, __LINE__, msg); \
		asm volatile("cli; hlt"); \
		while(1); \
	}

#endif