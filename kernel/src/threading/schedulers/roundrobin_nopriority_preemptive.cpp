
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "roundrobin_nopriority_preemptive.hpp"

#include <arch/hal.hpp>
#include <arch/threading.hpp>

using namespace threads;
using namespace threads::schedulers;

#define SCHED_LOCK(f)                                                                                                  \
	{                                                                                                                  \
		this->lock_scheduler();                                                                                        \
		{ f }                                                                                                          \
		this->unlock_scheduler();                                                                                      \
	}

void RoundRobinNoPriorityPreemptive::acquire_task(Task& new_task, priority_t) {
	SCHED_LOCK({
		this->ready_to_run_tasks.push_back(&new_task);
		if (this->ready_to_run_tasks.size() == 1) {
			// If only one task before adding new task, schedule to make sure new task gets time
			this->yield_internal();
		}
	})
}

void RoundRobinNoPriorityPreemptive::suspend_task(){SCHED_LOCK({ this->yield_internal(); })}

Task *RoundRobinNoPriorityPreemptive::get_current_task() {
	Task *ret = nullptr;
	SCHED_LOCK({ ret = this->current_task; })
	return ret;
}

Task *RoundRobinNoPriorityPreemptive::pop_task() {
	Task *ret = nullptr;
	SCHED_LOCK({
		if (!this->ready_to_run_tasks.empty()) {
			ret = this->ready_to_run_tasks.back();
			this->ready_to_run_tasks.pop_back();
		}
	})
	return ret;
}

void RoundRobinNoPriorityPreemptive::irq_fired() {}

void RoundRobinNoPriorityPreemptive::yield() {
	SCHED_LOCK({ this->yield_internal(); })
}

void RoundRobinNoPriorityPreemptive::yield_internal() {
	if (this->task_switch_disable_counter > 0) {
		// Return early if task switches locked
		this->task_switch_postponed = true;
		return;
	}

	if (!this->ready_to_run_tasks.empty()) {
		// Other tasks to schedule
		auto old_task = this->get_current_task();
		auto new_task = this->ready_to_run_tasks.front();
		this->ready_to_run_tasks.pop_front();

		if (old_task->get_state() == Task::State::RUNNING) {
			// If didn't block, then push back onto schedule queue
			this->ready_to_run_tasks.push_back(old_task);
		}
		this->task_switch(new_task);
	} else if (this->current_task->get_state() == Task::State::RUNNING) {
		// No other tasks to schedule, but old task not blocked
		// Therefore can just keep running it
		return;
	} else {
		// No tasks at all, go into idle
	}
}

void RoundRobinNoPriorityPreemptive::unlock_scheduler() {
	this->IRQ_disable_counter--;
	if (this->IRQ_disable_counter == 0) hal::enable_interrupts();
}

void RoundRobinNoPriorityPreemptive::lock_scheduler() {
	hal::disable_interrupts();
	this->IRQ_disable_counter++;
}
