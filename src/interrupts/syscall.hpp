#ifndef _HUGOS_SYSCALL_H
#define _HUGOS_SYSCALL_H

#include <stdint.h>

enum class syscall_vectors: uint64_t {
	yield,
	serial_write
};

int64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2);
int64_t syscall(syscall_vectors syscall_number, uint64_t arg1 = 0, uint64_t arg2 = 0);

#endif