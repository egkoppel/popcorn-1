#include "threading.hpp"

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

alignas(alignof(Scheduler)) char scheduler_[sizeof(Scheduler)];
Scheduler& threads::scheduler = reinterpret_cast<Scheduler&>(scheduler_);

extern "C" void task_init(void);
extern "C" void task_switch(Task *new_task, Task *old_task);
std::shared_ptr<Task> current_task_ptr;

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();
	
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	current_task_ptr = kernel_task;
	return kernel_task;
}

void Scheduler::schedule() {
	if (ready_to_run_tasks.size() == 0) return; // Only one task - don't need to switch

	auto old_task = current_task_ptr;
	auto new_task = ready_to_run_tasks.front();
	ready_to_run_tasks.pop_front();
	current_task_ptr = new_task;
	ready_to_run_tasks.push_back(old_task);

	task_switch(new_task.get(), old_task.get());
}

void Scheduler::add_task(std::shared_ptr<Task> task) {
	ready_to_run_tasks.push_back(task);
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
