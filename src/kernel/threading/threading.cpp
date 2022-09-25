#include "threading.hpp"
#include "../main/main.hpp"
#include <map>
#include <memory>
#include <stdio.h>

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

alignas(alignof(Scheduler)) char scheduler_[sizeof(Scheduler)];
Scheduler& threads::scheduler = reinterpret_cast<Scheduler&>(scheduler_);

extern "C" void task_switch_asm(Task *new_task, Task *old_task);

#define TIMER_FREQ (100)
static volatile uint64_t time_since_start_ms = 0;

std::shared_ptr<Task> Scheduler::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();

	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto init_task = std::make_shared<threads::Task>("uinit", cr3, stack_top, stack_top);
	scheduler.current_task_ptr = init_task;
	task_state_segment.privilege_stack_table[0] = init_task->get_kernel_stack().top;

	timer.set_frequency(TIMER_FREQ);

	return init_task;
}

void Scheduler::task_switch(std::shared_ptr<Task> task) {
	this->update_time_used();

	if (this->task_switch_disable_counter > 0) {
		this->task_switch_postponed = true;
		return;
	}

	std::swap(this->current_task_ptr, task);
	task_switch_asm(this->current_task_ptr.get(), task.get());
}

void Scheduler::schedule() {
	if (this->task_switch_disable_counter > 0) {
		this->task_switch_postponed = true;
		return;
	}

	if (!this->ready_to_run_tasks.empty()) { // If other tasks to switch to then switch to next one
		auto old_task = this->current_task_ptr;
		auto new_task = this->ready_to_run_tasks.front();
		this->ready_to_run_tasks.pop_front();

		if (old_task->get_state() == task_state::RUNNING) {
			this->ready_to_run_tasks.push_back(old_task);
			old_task->set_state(task_state::READY);
		}
		new_task->set_state(task_state::RUNNING);

		this->task_switch(new_task);
	} else if (this->current_task_ptr->get_state() == task_state::RUNNING) {
		// If no other tasks to switch to, and current task can stay return, then return
		return;
	} else {
		// No tasks to run - idle
		this->task_switch_disable_counter++; // Prevent task switch occuring while idle

		std::shared_ptr<Task> old_task(nullptr);
		std::swap(this->current_task_ptr, old_task); // Remove the no longer running task
		this->idle_time = 0;

		fprintf(stdserial, "idling\n");

		while (this->ready_to_run_tasks.empty()) {
			__asm__ volatile("sti");
			__asm__ volatile("hlt");
			__asm__ volatile("cli");
			this->update_time_used();
		}

		fprintf(stdserial, "idle for %dms\n", this->idle_time);

		// New task now exists, so switch to it
		auto new_task = this->ready_to_run_tasks.front();
		this->ready_to_run_tasks.pop_front();
		new_task->set_state(task_state::RUNNING);
		this->current_task_ptr = old_task; // Put the old task back for switching away from

		this->task_switch_disable_counter--; // Allow task switching to occur again

		if (new_task != old_task) this->task_switch(new_task); // If tasks are different, then switch to new task
	}
}

void Scheduler::add_task(const std::shared_ptr<Task>& task) {
	this->ready_to_run_tasks.push_back(task);
}

void Scheduler::block_task(task_state reason) {
	this->current_task_ptr->set_state(reason);
	this->schedule();
}

void Scheduler::unblock_task(const std::shared_ptr<Task>& task) {
	// Preempt if only one other task
	task->set_state(task_state::READY);
	this->ready_to_run_tasks.push_back(task);
	if (this->ready_to_run_tasks.size() == 1) {
		this->schedule();
	}
}

void Scheduler::sleep_until(uint64_t time) {
	this->sleep_queue.insert(decltype(this->sleep_queue)::value_type(time, this->current_task_ptr));
	fprintf(stdserial, "Sleeping until %lu - sleep queue size: %d\n", time, this->sleep_queue.size());
	block_task(task_state::SLEEPING);
}

Scheduler *SchedulerLock::operator ->() {
	return &scheduler;
}

SchedulerLock::SchedulerLock() {
	scheduler.lock_scheduler();
}

SchedulerLock SchedulerLock::get() {
	return {};
}

void Scheduler::__unlock_scheduler() {
	this->IRQ_disable_counter--;
	if (this->IRQ_disable_counter == 0) __asm__ volatile("sti");
}

void Scheduler::lock_scheduler() {
	__asm__ volatile("cli");
	this->IRQ_disable_counter++;
}

extern "C" void threads::unlock_scheduler_from_task_init() {
	scheduler.__unlock_scheduler();
}

SchedulerLock::~SchedulerLock() {
	this->operator ->()->__unlock_scheduler();
}

void Scheduler::irq() {
	time_since_start_ms += 1000 / TIMER_FREQ;
	uint64_t current_time = time_since_start_ms;
	//fprintf(stdserial, "time: %llu\n", current_time);

	scheduler.lock_task_switches();
	//fprintf(stdserial, "sleep queue len: %d\n", scheduler.sleep_queue.size());
	for (auto task = scheduler.sleep_queue.begin(); task != scheduler.sleep_queue.end(); task++) {
		//fprintf(stdserial, "wake time: %llu, time: %llu\n", task->first, current_time);
		if (task->first <= current_time) {
			fprintf(stdserial, "waking\n");
			scheduler.unblock_task(task->second);
			scheduler.sleep_queue.erase(task);
		}
	}
	scheduler.unlock_task_switches();
}

uint64_t threads::get_time_ms() {
	return time_since_start_ms;
}


void Scheduler::lock_task_switches() {
	this->lock_scheduler();
	this->task_switch_disable_counter++;
}

void Scheduler::unlock_task_switches() {
	this->task_switch_disable_counter--;
	if (this->task_switch_disable_counter == 0 && this->task_switch_postponed) {
		this->task_switch_postponed = false;
		this->schedule();
	}
	this->__unlock_scheduler();
}

void Scheduler::update_time_used() {
	auto current_time = get_time_ms();
	auto elapsed_time = current_time - this->last_time_used_update_time;
	this->last_time_used_update_time = current_time;
	if (!this->is_idle()) {
		this->current_task_ptr->time_used += elapsed_time;
	} else {
		this->idle_time += elapsed_time;
	}
}

uint64_t Scheduler::get_time_used() {
	this->update_time_used();
	return this->current_task_ptr->get_time_used();
}

std::shared_ptr<Task> Semaphore::get_next_waiting_task() {
	if (!this->waiting_tasks.empty()) {
		auto task = this->waiting_tasks.front();
		this->waiting_tasks.pop_front();
		return task;
	}
	return nullptr;
}

void Semaphore::wait() {
	scheduler.lock_task_switches();

	if (this->count == 0) {
		this->add_waiting_task(scheduler.current_task_ptr);
		scheduler.block_task(task_state::WAITING_FOR_LOCK);
	} else { this->count--; }

	scheduler.unlock_task_switches();
}

void Semaphore::post() {
	scheduler.lock_task_switches();

	if (this->count + 1 > this->max_count) return;

	if (auto task_to_wake = this->get_next_waiting_task()) {
		scheduler.unblock_task(task_to_wake);
	} else { this->count++; }

	scheduler.unlock_task_switches();
}

std::shared_ptr<Task> Mutex::get_next_waiting_task() {
	if (!this->waiting_tasks.empty()) {
		auto task = this->waiting_tasks.front();
		this->waiting_tasks.pop_front();
		return task;
	}
	return nullptr;
}

void Mutex::lock() {
	scheduler.lock_task_switches();

	if (this->locked) {
		this->add_waiting_task(scheduler.current_task_ptr);
		scheduler.block_task(task_state::WAITING_FOR_LOCK);
	} else { this->locked = true; }

	scheduler.unlock_task_switches();
}

uint64_t Mutex::try_lock() {
	scheduler.lock_task_switches();

	if (this->locked) { return 1; }
	else { this->locked = true; }

	scheduler.unlock_task_switches();
	return 0;
}

void Mutex::unlock() {
	scheduler.lock_task_switches();

	if (auto task_to_wake = this->get_next_waiting_task()) {
		scheduler.unblock_task(task_to_wake);
	} else { this->locked = false; }

	scheduler.unlock_task_switches();
}
