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
#include <map>
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
#define SYSCALL_ARGU(x) *reinterpret_cast<uint64_t *>(&(x))

static inline int64_t get_current_task() {
	auto local_data = get_local_data();
	return local_data->scheduler.get_current_task()->get_handle();
}
static inline int64_t yield() {
	auto local_data = get_local_data();
	local_data->scheduler.lock_scheduler();
	local_data->scheduler.schedule();
	local_data->scheduler.unlock_scheduler();
	return 0;
}
static inline int64_t exit(int64_t code) { return -1; }
static inline int64_t sleep(int64_t microseconds) {
	auto local_data = get_local_data();
	local_data->scheduler.sleep_ns(microseconds * 1000);
	return 0;
}
static inline int64_t suspend() {
	auto local_data = get_local_data();
	local_data->scheduler.block_task(threads::task_state::PAUSED);
	return 0;
}
static inline int64_t resume(syscall_handle_t handle) {
	auto local_data = get_local_data();
	//local_data->scheduler.unblock_task(threads::task_state::PAUSED);
	return -2;
}
static inline int64_t spawn(uint64_t name_, uint64_t entrypoint_, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	auto name = reinterpret_cast<char *>(name_);
	auto entrypoint = reinterpret_cast<void (*)(uint64_t, uint64_t, uint64_t)>(entrypoint_);

	auto local_data = get_local_data();
	auto new_proc = threads::new_proc(std::string(name), entrypoint, arg1, arg2, arg3);
	local_data->scheduler.add_task(new_proc);
	return new_proc->get_handle();
}
static inline int64_t get_time_used() {
	auto local_data = get_local_data();
	return local_data->scheduler.get_current_task()->get_time_used();
}

static inline int64_t mailbox_new() {
	return threads::new_mailbox(get_local_data()->scheduler.get_current_task());
}
static inline int64_t mailbox_send(syscall_handle_t handle, uint64_t timeout, uint64_t data_) {
	auto data = reinterpret_cast<void *>(data_);
	auto mbox = threads::get_mailbox(handle);
	if (!mbox) return -1;

	auto local_data = get_local_data();

	threads::message_t buf;
	memcpy(&buf.data, data, sizeof(buf.data));
	buf.sender = local_data->scheduler.get_current_task()->get_handle();

	auto recv_task = mbox->get_owning_task();
	auto send_success = mbox->send_message(&buf);
	if (send_success == 0 && recv_task->get_state() == threads::task_state::WAITING_FOR_MSG) local_data->scheduler.unblock_task(recv_task); // Unblock if waiting on message

	return send_success;
}
static inline int64_t mailbox_recv(syscall_handle_t handle, uint64_t timeout, uint64_t data_) {
	auto data = reinterpret_cast<threads::message_t *>(data_);
	auto mbox = threads::get_mailbox(handle);
	if (!mbox) return -1;

	auto local_data = get_local_data();

	if (mbox->get_message(data) == 0) return 0;
	local_data->scheduler.block_task(threads::task_state::WAITING_FOR_MSG);
	// Unlocked only if message arrived
	return (mbox->get_message(data));
}
static inline int64_t mailbox_destroy(syscall_handle_t handle) {
	threads::destroy_mailbox(handle);
	return 0;
}
static inline int64_t mailbox_transfer(syscall_handle_t mailbox_handle, syscall_handle_t task_handle) {
	auto mbox = threads::get_mailbox(mailbox_handle);
	if (!mbox) return -1;

	auto task = threads::task_handles_list.get_data_from_handle(task_handle, std::shared_ptr<threads::Task>(nullptr));
	if (!task) return -2;

	if (mbox->get_owning_task() != get_local_data()->scheduler.get_current_task()) return -3;

	mbox->set_owning_task(task);
	return 0;
}

int64_t syscall_handler(uint64_t syscall_number, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4, int64_t arg5) {
	switch (syscall_number) {
		// SCHEDULING
		case syscall_vectors::get_current_task: return get_current_task();
		case syscall_vectors::yield: return yield();
		case syscall_vectors::exit: return exit(arg1);
		case syscall_vectors::sleep: return sleep(arg1);
		case syscall_vectors::suspend: return suspend();
		case syscall_vectors::resume: return resume(arg1);
		case syscall_vectors::spawn: return spawn(SYSCALL_ARGU(arg1), SYSCALL_ARGU(arg2), SYSCALL_ARGU(arg3), SYSCALL_ARGU(arg4), SYSCALL_ARGU(arg5));
		case syscall_vectors::get_time_used: return get_time_used();

			// FUTEX

			// IPC

			// VM

			// IRQ
		case syscall_vectors::mailbox_new: return mailbox_new();
		case syscall_vectors::mailbox_send: return mailbox_send(arg1, SYSCALL_ARGU(arg2), SYSCALL_ARGU(arg3));
		case syscall_vectors::mailbox_recv: return mailbox_recv(arg1, SYSCALL_ARGU(arg2), SYSCALL_ARGU(arg3));
		case syscall_vectors::mailbox_destroy: return mailbox_destroy(arg1);
		case syscall_vectors::mailbox_transfer: return mailbox_transfer(arg1, arg2);

			// LEGACY SYSCALLS
		case syscall_vectors::mutex_lock: {
			((threads::Mutex *)arg1)->lock();
			return 0;
		}
		case syscall_vectors::mutex_unlock: {
			((threads::Mutex *)arg1)->unlock();
			return 0;
		}
		case syscall_vectors::mutex_try_lock: {
			return (((threads::Mutex *)arg1)->try_lock());
		}
		case syscall_vectors::mutex_new: {
			return ((uint64_t)(new threads::Mutex()));
		}
		case syscall_vectors::mutex_destroy: {
			delete (threads::Mutex *)arg1;
			return 0;
		}

		case syscall_vectors::sem_post: {
			((threads::Semaphore *)arg1)->post();
			return 0;
		}
		case syscall_vectors::sem_wait: {
			((threads::Semaphore *)arg1)->wait();
			return 0;
		}
		case syscall_vectors::sem_get_count: {
			return (((threads::Semaphore *)arg1)->get_count());
		}
		case syscall_vectors::sem_new: {
			return ((uint64_t)(new threads::Semaphore(arg1)));
		}
		case syscall_vectors::sem_destroy: {
			delete (threads::Semaphore *)arg1;
			return 0;
		}

			/*case syscall_vectors::mmap_anon: {
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
			case syscall_vectors::send_msg: {
				auto pid_to_send_to = arg1;
				auto get_buf = (threads::message_t *)arg2;
				get_buf->sender = local_data->scheduler.get_current_pid();

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
			}*/
		default: break;
	}

	return INT64_MIN;
}
