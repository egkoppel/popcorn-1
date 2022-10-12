/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_THREADING_H
#define _HUGOS_THREADING_H

#include <vector>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <assert.h>
#include <stdint.h>
#include <stdatomic.h>
#include <utils.h>
#include "../memory/stack.hpp"
#include "../memory/paging.h"
#include "../main/main.hpp"
#include "../interrupts/pit.hpp"
#include "../interrupts/syscall.hpp"
#include "../userspace/elf.hpp"
#include "../interrupts/idt.hpp"

namespace threads {
	uint64_t get_time_ms();

	extern "C" struct Task;
	extern atomic_uint_fast64_t next_pid;

	enum class task_state {
		RUNNING,
		READY,
		SLEEPING,
		WAITING_FOR_LOCK,
		PAUSED,
		WAITING_FOR_MSG
	};

	class Scheduler;

	uint64_t get_pid_by_name(const char *name);
	std::shared_ptr<Task> get_task_by_pid(uint64_t pid);

	extern "C" struct Task {
		friend std::shared_ptr<Task> std::make_shared<Task>(char const (&)[6], uint64_t&, uint64_t&, uint64_t&);

		friend class Scheduler;

		friend void::syscall_long_mode_handler();

	private:
		Stack code_stack;
		Stack kernel_stack;
		uint64_t stack_ptr;
		uint64_t p4_page_table;
		uint64_t pid = atomic_fetch_add(&next_pid, 1);
		std::string name;
		task_state state = task_state::READY;
		uint64_t time_used = 0;

		static const uint64_t userspace_stack_top = 0x7fff'ffff'ffff;
		static const uint64_t userspace_stack_size = 8 * 0x1000;

		Task(std::string name, uint64_t p4_page_table, uint64_t stack_top, uint64_t stack_bottom) :
				code_stack(userspace_stack_top, userspace_stack_top - userspace_stack_size + 1),
				kernel_stack(stack_top, stack_bottom),
				stack_ptr(0),
				p4_page_table(p4_page_table),
				name(std::move(name)),
				state(task_state::RUNNING) {

			entry_flags_t flags = {
					.writeable = true,
					.user_accessible = true,
					.write_through = false,
					.cache_disabled = false,
					.accessed = false,
					.dirty = false,
					.huge = false,
					.global = false,
					.no_execute = true,
			};

			for (uint64_t stack_addr = this->code_stack
			                               .bottom;
			     stack_addr < this->code_stack
			                      .top;
			     stack_addr += 0x1000) {
				map_page(stack_addr, flags, global_frame_allocator);
			}

			mark_for_no_map(this->code_stack
			                    .bottom - 0x1000, global_frame_allocator); // Guard page
		}

	public:
		Task(std::string name, uint64_t stack_value_count,
		     uint64_t p4_page_table = create_p4_table(global_frame_allocator)) :
				kernel_stack(4 * 0x1000),
				code_stack(userspace_stack_top, userspace_stack_top - userspace_stack_size + 1),
				stack_ptr(this->code_stack
				              .top - stack_value_count * 8),
				p4_page_table(p4_page_table),
				name(std::move(name)) {

			// Map kernel data into new process space
			map_kernel_from_current_into(p4_page_table, global_frame_allocator);

			// Create user stack in new process space
			auto ctx = mapper_ctx_begin(p4_page_table, global_frame_allocator);
			entry_flags_t flags = {
					.writeable = true,
					.user_accessible = true,
					.write_through = false,
					.cache_disabled = false,
					.accessed = false,
					.dirty = false,
					.huge = false,
					.global = false,
					.no_execute = true,
			};

			fprintf(stdserial, "code stack top: %p, bottom %p\n", this->code_stack
			                                                          .top, this->code_stack
			                                                                    .bottom);

			for (uint64_t stack_addr = this->code_stack
			                               .bottom;
			     stack_addr < this->code_stack
			                      .top;
			     stack_addr += 0x1000) {
				map_page(stack_addr, flags, global_frame_allocator);
			}

			mark_for_no_map(this->code_stack
			                    .bottom - 0x1000, global_frame_allocator); // Guard page

			mapper_ctx_end(ctx);
		}

	public:
		inline uint64_t get_pid() const { return this->pid; }
		inline std::string& get_name() { return this->name; }
		inline task_state get_state() const { return this->state; }
		inline void set_state(task_state state) { this->state = state; }
		inline uint64_t get_p4_page_table() const { return this->p4_page_table; }
		inline Stack& get_code_stack() { return this->code_stack; }
		inline Stack& get_kernel_stack() { return this->kernel_stack; }
		inline uint64_t get_time_used() const { return this->time_used; }
		inline uint64_t get_time_slice_length_ms() const { return 50; }
	};

	extern "C" void task_init(void);
	extern "C" void switch_to_user_mode(void);

	template<class... Args>
	static std::shared_ptr<Task> new_proc(std::string name, void(*entry_func)(Args...), Args... args) {
		auto task = std::make_shared<Task>(name, 9);
		auto& stack = task->get_code_stack();

		// Map new stack into current process address space
		auto ctx = mapper_ctx_begin(task->get_p4_page_table(), global_frame_allocator);
		uint64_t stack_frame;
		assert(translate_page(stack.top - 0x1000 + 1, &stack_frame) == 0);
		mapper_ctx_end(ctx);

		entry_flags_t flags = {
				.writeable = true,
				.user_accessible = false,
				.write_through = false,
				.cache_disabled = false,
				.accessed = false,
				.dirty = false,
				.huge = false,
				.global = false,
				.no_execute = true,
		};
		map_page_to(0xcafeb000, stack_frame, flags, global_frame_allocator);

		*((uint64_t *)0xcafebfff - 1) = reinterpret_cast<uint64_t>(entry_func);
		*((uint64_t *)0xcafebfff - 2) = reinterpret_cast<uint64_t>(switch_to_user_mode);
		*((uint64_t *)0xcafebfff - 3) = reinterpret_cast<uint64_t>(task_init);

		if constexpr (sizeof...(Args) > 0) {
			uint64_t args_list[sizeof...(Args)] = {reinterpret_cast<uint64_t>(args)...};

			if constexpr (sizeof...(Args) > 0) {
				*((uint64_t *)0xcafebfff - 4) = args_list[0];
			} // rbx - task_init arg 1
			if constexpr (sizeof...(Args) > 1) {
				*((uint64_t *)0xcafebfff - 5) = args_list[1];
			} // rbp - task_init arg 2
			if constexpr (sizeof...(Args) > 2) {
				*((uint64_t *)0xcafebfff - 6) = args_list[2];
			} // r12 - task_init arg 3
			if constexpr (sizeof...(Args) > 3) {
				*((uint64_t *)0xcafebfff - 7) = args_list[3];
			} // r13 - task_init arg 4
			if constexpr (sizeof...(Args) > 4) {
				*((uint64_t *)0xcafebfff - 8) = args_list[4];
			} // r14 - task_init arg 5
			if constexpr (sizeof...(Args) > 5) {
				*((uint64_t *)0xcafebfff - 9) = args_list[5];
			} // r15 - task_init arg 6
		}

		unmap_page_no_free(0xcafeb000);

		return task;
	}

	extern "C" void unlock_scheduler_from_task_init();
	std::shared_ptr<Task> init_multitasking(uint64_t stack_bottom, uint64_t stack_top);

	class Scheduler {
		friend void threads::unlock_scheduler_from_task_init();

		friend class Mutex;

		friend class Semaphore;

		friend void::syscall_long_mode_handler();

		friend std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top);

	private:
		std::shared_ptr<Task> current_task_ptr;
		std::deque<std::shared_ptr<Task>> ready_to_run_tasks;
		std::multimap<uint64_t /* wake time */, std::shared_ptr<Task>> sleep_queue;

		int IRQ_disable_counter = 0;
		int task_switch_disable_counter = 0;
		bool task_switch_postponed = false;

		uint64_t last_time_used_update_time = 0;
		uint64_t idle_time = 0;
		volatile uint64_t time_since_start_ns = 0;
		uint64_t time_left_for_current_task_ms = 0;

		void task_switch(std::shared_ptr<Task> task);
		void update_time_used();
		inline bool is_idle() { return current_task_ptr == nullptr; }

	public:
		void lock_task_switches();
		void unlock_task_switches();
		void add_task(const std::shared_ptr<Task>& task);
		void schedule();
		void block_task(task_state reason);
		void unblock_task(const std::shared_ptr<Task>& task);
		int unblock_task_by_pid(uint64_t);
		inline void sleep(uint64_t ms) { this->sleep_until(get_time_ns() + ms * 1000); }
		inline void sleep_ns(uint64_t ns) { this->sleep_until(get_time_ns() + ns); }
		void sleep_until(uint64_t time);
		void irq();
		uint64_t get_time_used();
		uint64_t get_time_ns() const { return this->time_since_start_ns; }
		void unlock_scheduler();
		void lock_scheduler();
		uint64_t get_current_pid() const { return this->current_task_ptr->get_pid(); }
	};
}

#endif
