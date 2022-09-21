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

uint64_t syscall_print(uint64_t arg1, uint64_t arg2) {
	return printf("%s", (char*)arg1);
}

union int_uint64_t { uint64_t u; int64_t i; };
#define SYSCALL_HANDLER_RET_I(x) { union int_uint64_t a { .i = x}; return a.u; }
#define SYSCALL_HANDLER_RET_U(x) { return x; }

uint64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	//fprintf(stdserial, "syscall, syscall_number: %llx, arg1: %llx, arg2: %llx\n", syscall_number, arg1, arg2);

	switch (syscall_number) {
		case syscall_vectors::yield: {
			threads::SchedulerLock::get()->schedule();
			return 0;
		}
		case syscall_vectors::serial_write: SYSCALL_HANDLER_RET_U(syscall_serial_write(arg1, arg2));
		case syscall_vectors::print: SYSCALL_HANDLER_RET_U(syscall_print(arg1, arg2));
		case syscall_vectors::get_time_used: SYSCALL_HANDLER_RET_U(threads::SchedulerLock::get()->get_time_used());
		case syscall_vectors::sleep: {
			threads::SchedulerLock::get()->sleep(arg1);
			SYSCALL_HANDLER_RET_U(0);
		}

		case syscall_vectors::mutex_lock: {
			((threads::Mutex*)arg1)->lock();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mutex_unlock: {
			((threads::Mutex*)arg1)->unlock();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mutex_try_lock: {
			SYSCALL_HANDLER_RET_U(((threads::Mutex*)arg1)->try_lock());
		}
		case syscall_vectors::mutex_new: {
			SYSCALL_HANDLER_RET_U((uint64_t)(new threads::Mutex()));
		}
		case syscall_vectors::mutex_destroy: {
			delete (threads::Mutex*)arg1;
			SYSCALL_HANDLER_RET_U(0);
		}

		case syscall_vectors::sem_post: {
			((threads::Semaphore*)arg1)->post();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::sem_wait: {
			((threads::Semaphore*)arg1)->wait();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::sem_get_count: {
			SYSCALL_HANDLER_RET_U(((threads::Semaphore*)arg1)->get_count());
		}
		case syscall_vectors::sem_new: {
			SYSCALL_HANDLER_RET_U((uint64_t)(new threads::Semaphore(arg1)));
		}
		case syscall_vectors::sem_destroy: {
			delete (threads::Semaphore*)arg1;
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mailbox_create: {
			mailboxes.push_back(mailbox_t { .write_index = static_cast<atomic_uint_fast64_t>(255), .read_index = static_cast<atomic_uint_fast64_t>(0) });
			SYSCALL_HANDLER_RET_U(mailboxes.size() - 1);
		}
		case syscall_vectors::new_proc: {
			auto new_proc = threads::new_proc(std::string((char*)arg1), (void(*)())arg2);
			threads::SchedulerLock::get()->add_task(new_proc);
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mmap_anon: {
			auto start_address = ALIGN_DOWN(arg1, 0x1000);
			auto size = ALIGN_UP(arg2, 0x1000);
			auto page_count = IDIV_ROUND_UP(size, 0x1000);
			fprintf(stdserial, "mmap(%llx, %llx) at %p, size %llx (%lld pages)\n", arg1, arg2, start_address, size, page_count);

			auto flags = arg2 & 0b1111;

			entry_flags_t page_flags = {
					.writeable = (bool)(flags & mmap_prot::PROT_WRITE),
					.user_accessible = true,
					.write_through = false,
					.cache_disabled = false,
					.accessed = false,
					.dirty = false,
					.huge = false,
					.global = false,
					.no_execute = !(bool)(flags & mmap_prot::PROT_EXEC),
			};

			for (uint64_t i = 0; i < page_count; i++) {
				map_page(start_address + i*0x1000, page_flags, global_frame_allocator);
			}

			SYSCALL_HANDLER_RET_U(start_address);
		}
	}

	SYSCALL_HANDLER_RET_I(-1);
}

uint64_t __attribute__((naked)) syscall(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2) {
	__asm__ volatile("syscall; ret;" :);
}
