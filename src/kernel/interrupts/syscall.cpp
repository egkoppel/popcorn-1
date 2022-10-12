/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "syscall.hpp"
#include "../threading/threading.hpp"
#include "../threading/mailing.hpp"
#include <stdio.h>
#include <vector>
#include <stdatomic.h>
#include "../threading/mutex.hpp"
#include "../threading/semaphore.hpp"
#include "../smp/core_local.hpp"

extern "C" __attribute__((naked)) void syscall_long_mode_handler() {
	// TODO: ****** should really switch to kernel stack here ******
	__asm__ volatile("swapgs"); // Switch to kernel gs
	__asm__ volatile("pushq %rcx"); // stores rip
	__asm__ volatile("pushq %r11"); // stores rflags
	__asm__ volatile("mov %r8, %rcx; mov %r9, %r8; mov %rax, %r9;");
	__asm__ volatile("call %P0" : : "i"(syscall_handler));
	__asm__ volatile("popq %r11");
	__asm__ volatile("popq %rcx");
	__asm__ volatile("swapgs"); // Switch back to user gs
	__asm__ volatile("sysretq");
}

uint64_t syscall_serial_write(uint64_t arg1, uint64_t arg2) {
	return fprintf(stdserial, "%s", (char *)arg1);
}

uint64_t syscall_print(uint64_t arg1, uint64_t arg2) {
	return printf("%s", (char *)arg1);
}

union int_uint64_t { uint64_t u; int64_t i; };
#define SYSCALL_HANDLER_RET_I(x) { union int_uint64_t a { .i = x}; return a.u; }
#define SYSCALL_HANDLER_RET_U(x) { return x; }

uint64_t syscall_handler(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
	auto local_data = get_local_data();

	switch (syscall_number) {
		case syscall_vectors::yield: {
			local_data->scheduler.lock_scheduler();
			local_data->scheduler.schedule();
			local_data->scheduler.unlock_scheduler();
			return 0;
		}
			//case syscall_vectors::serial_write: SYSCALL_HANDLER_RET_U(syscall_serial_write(arg1, arg2));
		case syscall_vectors::journal_log: SYSCALL_HANDLER_RET_U(syscall_serial_write(arg1, arg2));
		case syscall_vectors::get_time_used: SYSCALL_HANDLER_RET_U(local_data->scheduler.get_time_used());
		case syscall_vectors::sleep: {
			local_data->scheduler.sleep(arg1);
			SYSCALL_HANDLER_RET_U(0);
		}

		case syscall_vectors::mutex_lock: {
			((threads::Mutex *)arg1)->lock();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mutex_unlock: {
			((threads::Mutex *)arg1)->unlock();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mutex_try_lock: {
			SYSCALL_HANDLER_RET_U(((threads::Mutex *)arg1)->try_lock());
		}
		case syscall_vectors::mutex_new: {
			SYSCALL_HANDLER_RET_U((uint64_t)(new threads::Mutex()));
		}
		case syscall_vectors::mutex_destroy: {
			delete (threads::Mutex *)arg1;
			SYSCALL_HANDLER_RET_U(0);
		}

		case syscall_vectors::sem_post: {
			((threads::Semaphore *)arg1)->post();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::sem_wait: {
			((threads::Semaphore *)arg1)->wait();
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::sem_get_count: {
			SYSCALL_HANDLER_RET_U(((threads::Semaphore *)arg1)->get_count());
		}
		case syscall_vectors::sem_new: {
			SYSCALL_HANDLER_RET_U((uint64_t)(new threads::Semaphore(arg1)));
		}
		case syscall_vectors::sem_destroy: {
			delete (threads::Semaphore *)arg1;
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::spawn: {
			auto new_proc = threads::new_proc(std::string((char *)arg1), (void (*)(uint64_t, uint64_t, uint64_t))arg2, (uint64_t)arg3, (uint64_t)arg4, (uint64_t)arg5);
			local_data->scheduler.add_task(new_proc);
			SYSCALL_HANDLER_RET_U(0);
		}
		case syscall_vectors::mmap_anon: {
			auto start_address = ALIGN_DOWN(arg1, 0x1000);
			auto size = ALIGN_UP(arg2, 0x1000);
			auto page_count = IDIV_ROUND_UP(size, 0x1000);
			fprintf(stdserial, "mmap(%llx, %llx) at %p, size %llx (%lld pages)\n", arg1, arg2, start_address, size,
			        page_count);

			entry_flags_t page_flags = {
					.writeable = (bool)(arg3 & mmap_flags::PROT_WRITE),
					.user_accessible = true,
					.write_through = false,
					.cache_disabled = false,
					.accessed = false,
					.dirty = false,
					.huge = false,
					.global = false,
					.no_execute = !(bool)(arg3 & mmap_flags::PROT_EXEC),
			};

			fprintf(stdserial, "mmap flags w:%d, nx:%d\n", (bool)(arg3 & mmap_flags::PROT_WRITE), !(bool)(arg3 & mmap_flags::PROT_EXEC));

			for (uint64_t i = 0; i < page_count; i++) {
				map_page(start_address + i * 0x1000, page_flags, global_frame_allocator);
			}

			SYSCALL_HANDLER_RET_U(start_address);
		}
		case syscall_vectors::get_pid_by_name: {
			auto name = (const char *)arg1;
			SYSCALL_HANDLER_RET_U(threads::get_pid_by_name(name));
		}
		case syscall_vectors::send_msg: {
			auto pid_to_send_to = arg1;
			auto get_buf = (threads::message_t *)arg2;

			if (auto mailbox = threads::get_mailbox(pid_to_send_to); mailbox != nullptr) {
				auto task = threads::get_task_by_pid(pid_to_send_to);
				auto send_success = mailbox->send_message(get_buf);
				if (send_success == 0 && task->get_state() == threads::task_state::WAITING_FOR_MSG) local_data->scheduler.unblock_task_by_pid(pid_to_send_to); // Unblock if waiting on message
				SYSCALL_HANDLER_RET_I(send_success);
			} else SYSCALL_HANDLER_RET_I(-2);
		}
		case syscall_vectors::get_msg: {
			auto store_buf = (threads::message_t *)arg1;
			SYSCALL_HANDLER_RET_I(threads::get_mailbox(local_data->scheduler.get_current_pid())->get_message(store_buf));
		}
		case syscall_vectors::wait_msg: {
			auto store_buf = (threads::message_t *)arg1;
			if (threads::get_mailbox(local_data->scheduler.get_current_pid())->get_message(store_buf) == 0) SYSCALL_HANDLER_RET_I(0);
			get_local_data()->scheduler.block_task(threads::task_state::WAITING_FOR_MSG);
			// Unlocked only if message arrived
			SYSCALL_HANDLER_RET_I(threads::get_mailbox(local_data->scheduler.get_current_pid())->get_message(store_buf));
		}
		default: break;
	}

	SYSCALL_HANDLER_RET_I(-1);
}
