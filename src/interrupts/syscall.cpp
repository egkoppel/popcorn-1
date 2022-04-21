#include "syscall.hpp"
#include <stdio.h>

uint64_t syscall_serial_write(uint64_t arg1, uint64_t arg2) {
	return fprintf(stdserial, "%s", (char*)arg1);
}

int64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	fprintf(stdserial, "syscall, syscall_number: %llx, arg1: %llx, arg2: %llx\n", syscall_number, arg1, arg2);

	switch (syscall_number) {
		case syscall_vectors::serial_write: return syscall_serial_write(arg1, arg2);
	}

	return -1;
}

int64_t syscall(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	int64_t ret;
	__asm__ volatile("syscall" : "=a"(ret) : "D"(syscall_number), "S"(arg1), "d"(arg2));
	return ret;
}
