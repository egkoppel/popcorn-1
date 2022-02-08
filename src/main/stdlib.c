#include <stdint.h>

static char itoa_str[] = "0123456789abcdefghijklmnopqrstuvwxyz";

char* itoa(uint64_t val, uint64_t base) {
	static char buf[32] = {0};
	int i = 30;
	
	for(; val && i ; --i, val /= base)
		buf[i] = itoa_str[val % base];
	
	return &buf[i+1];
}
