#include "threading.hpp"

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

alignas(alignof(Scheduler)) char scheduler_[sizeof(Scheduler)];
Scheduler& threads::scheduler = reinterpret_cast<Scheduler&>(scheduler_);

extern "C" void task_init(void);
extern "C" void task_switch_asm(Task *new_task, Task *old_task);

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();
	
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	current_task_ptr = kernel_task;
	return kernel_task;
}

void Scheduler::task_switch(std::shared_ptr<Task> task) {
	if (this->task_switch_disable_counter > 0) {
		this->task_switch_postponed = true;
		return;
	}

	std::swap(this->current_task_ptr, task);
	task_switch_asm(this->current_task_ptr.get(), task.get());
}

void Scheduler::schedule() {
	if (ready_to_run_tasks.size() == 0) return; // Only one task - don't need to switch

	auto old_task = this->current_task_ptr;
	auto new_task = this->ready_to_run_tasks.front();
	this->ready_to_run_tasks.pop_front();
	this->current_task_ptr = new_task;
	if (old_task->get_state() == task_state::RUNNING) {
		this->ready_to_run_tasks.push_back(old_task);
		old_task->set_state(task_state::READY);
	}
	new_task->set_state(task_state::RUNNING);

	this->task_switch(new_task);
}

void Scheduler::add_task(std::shared_ptr<Task> task) {
	ready_to_run_tasks.push_back(task);
}

void Scheduler::block_task(task_state reason) {
	current_task_ptr->set_state(reason);
	schedule();
}

void Scheduler::unblock_task(std::shared_ptr<Task> task) {
	// Preempt if only one other task
	task->set_state(task_state::READY);
	ready_to_run_tasks.push_back(task);
	if (ready_to_run_tasks.size() == 1) {
		schedule();
	}
}

Scheduler* SchedulerLock::operator->() {
	return &scheduler;
}

SchedulerLock::SchedulerLock() {
	__asm__ volatile("cli");
	scheduler.IRQ_disable_counter++;
}

SchedulerLock SchedulerLock::get() {
	return SchedulerLock();
}

void Scheduler::__unlock_scheduler() {
	this->IRQ_disable_counter--;
	if (this->IRQ_disable_counter == 0) __asm__ volatile("sti");
}

extern "C" void threads::unlock_scheduler_from_task_init() {
	scheduler.__unlock_scheduler();
}

SchedulerLock::~SchedulerLock() {
	this->operator->()->__unlock_scheduler();
}
