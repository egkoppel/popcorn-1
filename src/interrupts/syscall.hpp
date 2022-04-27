#ifndef _HUGOS_SYSCALL_H
#define _HUGOS_SYSCALL_H

#include <stdint.h>

enum class syscall_vectors: uint64_t {
	yield,
	serial_write,
	get_time_used,
	sleep,

	mutex_lock,
	mutex_try_lock,
	mutex_unlock,
	mutex_new,
	mutex_destroy,

	sem_post,
	sem_wait,
	sem_get_count,
	sem_new,
	sem_destroy
};

int64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2);
int64_t syscall(syscall_vectors syscall_number, uint64_t arg1 = 0, uint64_t arg2 = 0);

#endif
