#ifndef _HUGOS_THREADING_H
#define _HUGOS_THREADING_H

#include <vector>
#include <memory>
#include <string>
#include <stdint.h>
#include <stdatomic.h>
#include "../memory/stack.hpp"
#include "../memory/paging.h"
#include "../main/main.h"

namespace threads {
	extern "C" struct Task;
	extern atomic_uint_fast64_t next_pid;

	std::shared_ptr<Task> init_multitasking(uint64_t stack_bottom, uint64_t stack_top);

	enum class task_state {
		RUNNING
	};

	extern "C" struct Task {
		private:
		Stack stack;
		uint64_t stack_ptr;
		uint64_t p4_page_table;
		uint64_t pid;
		std::string name;
		task_state state;

		public:
		Task(std::string name, uint64_t p4_page_table = create_p4_table(global_frame_allocator)): pid(atomic_fetch_add(&next_pid, 1)), name(std::move(name)), state(task_state::RUNNING), stack(40*1024), p4_page_table(p4_page_table), stack_ptr(this->stack.top - 8*8 /* 8 uint64_t get popped off on entry - 6 regs + 2 return */) {}
		Task(std::string name, uint64_t p4_page_table, uint64_t stack_top, uint64_t stack_bottom): pid(atomic_fetch_add(&next_pid, 1)), name(std::move(name)), state(task_state::RUNNING), stack(stack_top, stack_bottom), p4_page_table(p4_page_table), stack_ptr(0) {}

		public:
		uint64_t get_pid() { return pid; }
		std::string& get_name() { return name; }
		task_state get_state() { return state; }
		uint64_t get_p4_page_table() { return p4_page_table; }
		Stack& get_stack() { return stack; }

		static std::shared_ptr<Task> new_kernel_task(std::string name, void(*entry_func)(uint64_t, uint64_t, uint64_t), uint64_t arg1, uint64_t arg2, uint64_t arg3);
	};

	class SchedulerLock;
	extern "C" void unlock_scheduler_from_task_init();

	class Scheduler {
		friend class SchedulerLock;
		friend void unlock_scheduler_from_task_init();
		
		private:
		std::vector<std::shared_ptr<Task>> tasks;
		std::vector<std::shared_ptr<Task>> running_tasks;
		size_t current_task_index = 0;
		int IRQ_disable_counter = 0;
		
		void __unlock_scheduler();

		public:
		void print_tasks();
		void add_task(std::shared_ptr<Task>);
		void schedule();
	};

	class SchedulerLock {
		private:
		SchedulerLock();

		public:
		static SchedulerLock get();
		~SchedulerLock();
		
		Scheduler* operator->();
	};

	extern Scheduler& scheduler;
}

#endif
