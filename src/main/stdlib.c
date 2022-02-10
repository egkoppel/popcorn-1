#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>

#define ATOI_BUFLEN 32

static const char itoa_str[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
char* itoa(int64_t val, char *str, int base) {
	bool neg = val < 0;
	if (neg) val *= -1; // abs
	
	static char buf[ATOI_BUFLEN] = {0};
	int i = ATOI_BUFLEN - 2; // leave final char as '\0'
	
	do {
		buf[i] = itoa_str[val % base];
		
		--i;
		val /= base;
	} while(val != 0 && i >= 0);
	
	if (neg && i >= 0) {
		buf[i--] = '-';
	}
	
	memcpy(str, &buf[i+1], ATOI_BUFLEN - i - 1);
	
	return str;
}

int atoi_p(const char **s) {
	int n = 0;
	while (**s >= '0' && **s <= '9') {
		n = n*10 + (*(*s)++ - '0');
	}
	return n;
}

int atoi(const char *s) {
	int n = 0;
	while (*s >= '0' && *s <= '9') {
		n = n*10 + *s++ - '0';
	}
	return n;
}
