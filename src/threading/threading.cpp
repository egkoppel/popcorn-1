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

std::shared_ptr<Task> Task::new_kernel_task(std::string name, void(*entry_func)(uint64_t, uint64_t, uint64_t), uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto task = std::make_shared<Task>(name, cr3);
	auto& stack = task->get_stack();

	*((uint64_t*)stack.top - 1) = (uint64_t)entry_func;
	*((uint64_t*)stack.top - 2) = (uint64_t)task_init;
	*((uint64_t*)stack.top - 3) = (uint64_t)0; // rbx
	*((uint64_t*)stack.top - 4) = (uint64_t)0; // rbp
	*((uint64_t*)stack.top - 5) = (uint64_t)0; // r12
	*((uint64_t*)stack.top - 6) = (uint64_t)arg1; // r13 - task_init arg 1
	*((uint64_t*)stack.top - 7) = (uint64_t)arg2; // r14 - task_init arg 2
	*((uint64_t*)stack.top - 8) = (uint64_t)arg3; // r15 - task_init arg 3
	return task;
}

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();
	
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	current_task_ptr = kernel_task.get();
	scheduler.lock().add_task(kernel_task);
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

SchedulerLock Scheduler::lock() {
	return SchedulerLock(this);
}

SchedulerLock::SchedulerLock(Scheduler *s): s(s) {
	__asm__ volatile("cli");
	s->IRQ_disable_counter++;
}

SchedulerLock::~SchedulerLock() {
	s->IRQ_disable_counter--;
	if (s->IRQ_disable_counter == 0) __asm__ volatile("sti");
}

void SchedulerLock::print_tasks() { s->print_tasks(); }
void SchedulerLock::add_task(std::shared_ptr<Task> task) { s->add_task(task); }
void SchedulerLock::schedule() { s->schedule(); }
