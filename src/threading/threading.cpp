#include "threading.hpp"
#include <map>

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

alignas(alignof(Scheduler)) char scheduler_[sizeof(Scheduler)];
Scheduler& threads::scheduler = reinterpret_cast<Scheduler&>(scheduler_);

extern "C" void task_init(void);
extern "C" void task_switch_asm(Task *new_task, Task *old_task);

#define TIMER_FREQ (100)
static volatile uint64_t time_since_start_ms = 0;

std::shared_ptr<Task> Scheduler::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new(&threads::scheduler) Scheduler();
	
	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto kernel_task = std::make_shared<threads::Task>("kernel_task", cr3, stack_top, stack_top);
	scheduler.current_task_ptr = kernel_task;

	timer.set_frequency(TIMER_FREQ);

	return kernel_task;
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
	fprintf(stdserial, "schedule()\n");
	if (this->ready_to_run_tasks.size() == 0) {
		fprintf(stdserial, "schedule(): no other tasks to run\n");
		return; // Only one task - don't need to switch
	}

	if (this->task_switch_disable_counter > 0) {
		fprintf(stdserial, "schedule(): task switch disabled\n");
		this->task_switch_postponed = true;
		return;
	}

	auto old_task = this->current_task_ptr;
	auto new_task = this->ready_to_run_tasks.front();
	this->ready_to_run_tasks.pop_front();
	//this->current_task_ptr = new_task;
	if (old_task->get_state() == task_state::RUNNING) {
		this->ready_to_run_tasks.push_back(old_task);
		old_task->set_state(task_state::READY);
	}
	new_task->set_state(task_state::RUNNING);

	this->task_switch(new_task);
}

void Scheduler::add_task(std::shared_ptr<Task> task) {
	this->ready_to_run_tasks.push_back(task);
}

void Scheduler::block_task(task_state reason) {
	this->current_task_ptr->set_state(reason);
	this->schedule();
}

void Scheduler::unblock_task(std::shared_ptr<Task> task) {
	// Preempt if only one other task
	task->set_state(task_state::READY);
	this->ready_to_run_tasks.push_back(task);
	if (this->ready_to_run_tasks.size() == 1) {
		this->schedule();
	}
}

void Scheduler::sleep(uint64_t ms) {
	this->sleep_until(get_time_ms() + ms);
}

void Scheduler::sleep_until(uint64_t time) {
	this->sleep_queue.insert(decltype(this->sleep_queue)::value_type(time, this->current_task_ptr));
	fprintf(stdserial, "Sleeping until %lu - sleep queue size: %d\n", time, this->sleep_queue.size());
	block_task(task_state::SLEEPING);
}

Scheduler* SchedulerLock::operator->() {
	return &scheduler;
}

SchedulerLock::SchedulerLock() {
	scheduler.lock_scheduler();
}

SchedulerLock SchedulerLock::get() {
	return SchedulerLock();
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
	this->operator->()->__unlock_scheduler();
}

void Scheduler::irq() {
	time_since_start_ms += 1000/TIMER_FREQ;
	uint64_t current_time = time_since_start_ms;
	fprintf(stdserial, "time: %llu\n", current_time);

	scheduler.lock_task_switches();
	fprintf(stdserial, "sleep queue len: %d\n", scheduler.sleep_queue.size());
	for (auto task = scheduler.sleep_queue.begin(); task != scheduler.sleep_queue.end(); task++) {
		fprintf(stdserial, "wake time: %llu, time: %llu\n", task->first, current_time);
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
	this->current_task_ptr->time_used += elapsed_time;
}

uint64_t Scheduler::get_time_used() {
	this->update_time_used();
	return this->current_task_ptr->get_time_used();
}
