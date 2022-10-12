/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "threading.hpp"
#include "../main/main.hpp"
#include <map>
#include <memory>
#include <stdio.h>
#include "../smp/core_local.hpp"
#include "../amd64_macros.hpp"
#include "mailing.hpp"

using namespace threads;

atomic_uint_fast64_t threads::next_pid = 1;

extern "C" void task_switch_asm(Task *new_task, Task *old_task);

alignas(alignof(std::map<uint64_t, std::shared_ptr<Task>>)) static char task_list_[sizeof(std::map<uint64_t, std::shared_ptr<Task>>)]; // memory for the stream object
auto& task_list = reinterpret_cast<std::map<uint64_t, std::shared_ptr<Task>>&>(task_list_);

#define TIMER_FREQ (1000)

std::shared_ptr<Task> threads::init_multitasking(uint64_t stack_bottom, uint64_t stack_top) {
	new core_local();
	new(&task_list) std::map<uint64_t, std::shared_ptr<Task>>();
	new(&mailboxes) std::map<uint64_t, std::unique_ptr<Mailbox>>();

	uint64_t cr3;
	__asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
	auto init_task = std::make_shared<threads::Task>("uinit", cr3, stack_top, stack_top);
	task_list.insert({init_task->get_pid(), init_task});
	new_mailbox(init_task->get_pid());
	get_local_data()->scheduler.current_task_ptr = init_task;
	get_local_data()->scheduler.time_left_for_current_task_ms = init_task->get_time_slice_length_ms();
	task_state_segment.privilege_stack_table[0] = init_task->get_kernel_stack().top;

	timer.set_frequency(TIMER_FREQ);
	sti();

	return init_task;
}

void Scheduler::task_switch(std::shared_ptr<Task> task) {
	this->update_time_used();

	if (this->task_switch_disable_counter > 0) {
		this->task_switch_postponed = true;
		return;
	}

	this->time_left_for_current_task_ms = task->get_time_slice_length_ms();

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
			sti();
			hlt();
			cli();
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
	this->lock_scheduler();
	this->ready_to_run_tasks.push_back(task);
	task_list.insert({task->get_pid(), task});
	new_mailbox(task->get_pid());
	if (this->ready_to_run_tasks.size() == 1) {
		// If only one task before adding new task, schedule to make sure new task gets time
		this->schedule();
	}
	this->unlock_scheduler();
}

void Scheduler::block_task(task_state reason) {
	this->lock_scheduler();
	this->current_task_ptr->set_state(reason);
	this->schedule();
	this->unlock_scheduler();
}

void Scheduler::unblock_task(const std::shared_ptr<Task>& task) {
	this->lock_scheduler();
	// Preempt if only one other task
	task->set_state(task_state::READY);
	this->ready_to_run_tasks.push_back(task);
	if (this->ready_to_run_tasks.size() == 1) {
		this->schedule();
	}
	this->unlock_scheduler();
}

void Scheduler::sleep_until(uint64_t time) {
	this->lock_scheduler();
	this->sleep_queue.insert(decltype(this->sleep_queue)::value_type(time, this->current_task_ptr));
	fprintf(stdserial, "Sleeping until %lu - sleep queue size: %d\n", time, this->sleep_queue.size());
	this->unlock_scheduler();
	block_task(task_state::SLEEPING);
}

void Scheduler::unlock_scheduler() {
	this->IRQ_disable_counter--;
	if (this->IRQ_disable_counter == 0) sti();
}

void Scheduler::lock_scheduler() {
	cli();
	this->IRQ_disable_counter++;
}

extern "C" void threads::unlock_scheduler_from_task_init() {
	get_local_data()->scheduler.unlock_scheduler();
}

void Scheduler::irq() {
	this->lock_task_switches();

	constexpr uint64_t ms_between_ticks = 1000 / TIMER_FREQ;
	this->time_since_start_ns += 1000 * ms_between_ticks;
	uint64_t current_time = this->time_since_start_ns;

	// Wake up sleeping tasks
	for (auto task = this->sleep_queue.begin(); task != this->sleep_queue.end(); task++) {
		//fprintf(stdserial, "wake time: %llu, time: %llu\n", task->first, current_time);
		if (task->first <= current_time) {
			fprintf(stdserial, "waking\n");
			this->unblock_task(task->second);
			this->sleep_queue.erase(task);
		}
	}

	// Reschedule if time slice gone
	if (this->time_left_for_current_task_ms < ms_between_ticks) {
		schedule(); // Schedule new task if time slice gone
	} else {
		this->time_left_for_current_task_ms -= ms_between_ticks;
	}

	this->unlock_task_switches();
}

uint64_t threads::get_time_ms() {
	return get_local_data()->scheduler.get_time_ns() * 1000;
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
	this->unlock_scheduler();
}

void Scheduler::update_time_used() {
	this->lock_scheduler();
	auto current_time = get_time_ms();
	auto elapsed_time = current_time - this->last_time_used_update_time;
	this->last_time_used_update_time = current_time;
	if (!this->is_idle()) {
		this->current_task_ptr->time_used += elapsed_time;
	} else {
		this->idle_time += elapsed_time;
	}
	this->unlock_scheduler();
}

uint64_t Scheduler::get_time_used() {
	this->update_time_used();
	return this->current_task_ptr->get_time_used();
}

uint64_t threads::get_pid_by_name(const char *name) {
	auto name_equal = [name](std::pair<const uint64_t, std::shared_ptr<Task>> i) { return i.second->get_name() == name; };

	if (auto task = std::find_if(task_list.begin(), task_list.end(), name_equal); task != task_list.end()) {
		return task->first;
	} else {
		return 0;
	}
}

std::shared_ptr<Task> threads::get_task_by_pid(uint64_t pid) {
	if (auto task = task_list.find(pid); task != task_list.end()) {
		return task->second;
	} else {
		return {nullptr};
	}
}

int Scheduler::unblock_task_by_pid(uint64_t pid) {
	if (auto task = task_list.find(pid); task != task_list.end()) {
		this->unblock_task(task->second);
	} else return -1;
}
