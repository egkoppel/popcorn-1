#ifndef _HUGOS_PANIC_H
#define _HUGOS_PANIC_H

#include <termcolor.h>
#include <stdio.h>

#define panic(msg, ...) \
	{ \
		printf("[" TERMCOLOR_RED "ERR!" TERMCOLOR_RESET "]" TERMCOLOR_RED " Kernel panicked at %s:%d:\n\t", __FILE__, __LINE__); \
		printf(msg, ##__VA_ARGS__); \
		printf("\n"); \
		__asm__ volatile("cli; hlt"); \
		while(1); \
	}

#endif
