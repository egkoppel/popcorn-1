#include "syscall.hpp"
#include "../threading/threading.hpp"
#include <stdio.h>
#include <vector>
#include <stdatomic.h>

extern "C" void fast_message_send();

const int MAX_MESSAGES = 256;

struct incoming_message_t {
	uint64_t sender_pid;
	uint64_t message;
};

struct mailbox_t {
	incoming_message_t messages[MAX_MESSAGES];
	atomic_uint_fast64_t write_index;
	atomic_uint_fast64_t read_index;
};
std::vector<mailbox_t> mailboxes;

/*static inline int64_t send_message() {
	uint64_t current_pid = threads::scheduler.get
}*/

extern "C" __attribute__((naked)) void syscall_long_mode_handler() {
	// ****** should really switch to kernel stack here ******
	__asm__ volatile("pushq %rcx"); // stores rip
	__asm__ volatile("pushq %r11"); // stores rflags
	__asm__ volatile("call %P0" : : "i"(syscall_handler));
	__asm__ volatile("popq %r11");
	__asm__ volatile("popq %rcx");
	__asm__ volatile("sysretq");
}

/* see `message.S.no_compile` for clearer nasm code for message passing */
/*extern "C" __attribute__((naked)) void syscall_long_mode_handler() {
	__asm__ volatile("pushq %%rcx; # stores rip \n\
	                  pushq %%r11; # stores rflafs" : : : "memory");
	__asm__ volatile("cmp %0, %%rdi; \n\
	                  jne 1f;\n\
					  movq %%rax, %%r11 \n\
					  movq %%rdx, %%r10 \n\
					  movabs %2, %%rax \n\
					  mul %%rsi \n\
					  add %%r11, %%rax \n\
					  movq %P4(%%rax), %%r9 \n\
					  cmp %7, %%r9 \n\
					  jl 2f; \n\
					  	movabs $-1, %%rax \n\
						popq %%r11; \n\
						popq %%rcx; \n\
						sysretq; # failed to send: message queue full \n\
					  2: \n\
					  	lock incq %P4(%%rax); \n\
						movq %%rax, %%r11; \n\
						movabs %3, %%rax; \n\
						mul %%r9; \n\
						addq %%r11, %%rax; \n\
						movq %%rcx, %P5(%%rax); \n\
						movq %%r10, %P6(%%rax); \n\
						movabs $0, %%rax; \n\
						popq %%r11; \n\
						popq %%rcx; \n\
						sysretq; # success \n\
	                  1:; \n\
	                  call %P1; \n\
	                  popq %%r11; \n\
	                  popq %%rcx; \n\
	                  sysretq;" : : "i"(syscall_vectors::message_send), "i"(syscall_handler),
	                  "i"(sizeof(mailbox_t)), "i"(sizeof(incoming_message_t)), "i"(offsetof(mailbox_t, write_index)), "i"(offsetof(incoming_message_t, sender_pid)), "i"(offsetof(incoming_message_t, message)), "i"(MAX_MESSAGES),
	                  "c"(threads::scheduler.current_task_ptr.ptr->pid), "a"(mailboxes._M_start));
}*/

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
