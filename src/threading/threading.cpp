#include "threading.hpp"

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

alignas(alignof(Scheduler)) char scheduler_[sizeof(Scheduler)];
Scheduler& threads::scheduler = reinterpret_cast<Scheduler&>(scheduler_);

extern "C" void task_init(void);
extern "C" void task_switch(Task *new_task);
extern "C" Task* current_task_ptr = nullptr;

void Scheduler::print_tasks() {
	for (auto& task : tasks) {
		printf("%lli: %s\n", task->get_pid(), task->get_name().c_str());
	}
}

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();
	
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	current_task_ptr = kernel_task.get();
	SchedulerLock::get()->add_task(kernel_task);
	return kernel_task;
}

void Scheduler::schedule() {
	if (running_tasks.size() == 1) return; // Only one task - don't need to switch
	current_task_index++;
	if (current_task_index >= running_tasks.size()) current_task_index = 0;
	task_switch(running_tasks[current_task_index].get());
}

void Scheduler::add_task(std::shared_ptr<Task> task) {
	tasks.push_back(task);
	running_tasks.push_back(task);
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
