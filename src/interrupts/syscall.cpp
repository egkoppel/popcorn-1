#include "syscall.hpp"
#include "../threading/threading.hpp"
#include <stdio.h>

uint64_t syscall_serial_write(uint64_t arg1, uint64_t arg2) {
	return fprintf(stdserial, "%s", (char*)arg1);
}

int64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	//fprintf(stdserial, "syscall, syscall_number: %llx, arg1: %llx, arg2: %llx\n", syscall_number, arg1, arg2);

	switch (syscall_number) {
		case syscall_vectors::yield: {
			threads::SchedulerLock::get()->schedule();
			return 0;
		}
		case syscall_vectors::serial_write: return syscall_serial_write(arg1, arg2);
		case syscall_vectors::get_time_used: return threads::SchedulerLock::get()->get_time_used();
		case syscall_vectors::sleep: {
			threads::SchedulerLock::get()->sleep(arg1);
			return 0;
		}

		case syscall_vectors::mutex_lock: {
			((threads::Mutex*)arg1)->lock();
			return 0;
		}
		case syscall_vectors::mutex_unlock: {
			((threads::Mutex*)arg1)->unlock();
			return 0;
		}
		case syscall_vectors::mutex_try_lock: {
			return ((threads::Mutex*)arg1)->try_lock();
		}
		case syscall_vectors::mutex_new: {
			return (uint64_t)(new threads::Mutex());
		}
		case syscall_vectors::mutex_destroy: {
			delete (threads::Mutex*)arg1;
			return 0;
		}

		case syscall_vectors::sem_post: {
			((threads::Semaphore*)arg1)->post();
			return 0;
		}
		case syscall_vectors::sem_wait: {
			((threads::Semaphore*)arg1)->wait();
			return 0;
		}
		case syscall_vectors::sem_get_count: {
			return ((threads::Semaphore*)arg1)->get_count();
		}
		case syscall_vectors::sem_new: {
			return (uint64_t)(new threads::Semaphore(arg1));
		}
		case syscall_vectors::sem_destroy: {
			delete (threads::Semaphore*)arg1;
			return 0;
		}
	}

	return -1;
}

int64_t syscall(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	int64_t ret;
	__asm__ volatile("syscall" : "=a"(ret) : "D"(syscall_number), "S"(arg1), "d"(arg2));
	return ret;
}
