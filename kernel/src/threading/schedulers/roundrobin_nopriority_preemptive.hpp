
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_ROUNDROBIN_NOPRIORITY_PREEMPTIVE_HPP
#define HUGOS_ROUNDROBIN_NOPRIORITY_PREEMPTIVE_HPP

#include "../scheduler.hpp"

#include <deque>
#include <stdint.h>

namespace threads::schedulers {
	class RoundRobinNoPriorityPreemptive : public ILocalScheduler {
	private:
		Task *current_task;
		std::deque<Task *> ready_to_run_tasks;

		int IRQ_disable_counter         = 0;
		int task_switch_disable_counter = 0;
		bool task_switch_postponed      = false;

		uint64_t last_time_used_update_time    = 0;
		uint64_t idle_time                     = 0;
		volatile uint64_t time_since_start_ns  = 0;
		uint64_t time_left_for_current_task_ms = 0;

		void task_switch(Task *task);
		void update_time_used();
		inline bool is_idle() { return !current_task; }
		void lock_scheduler();
		void unlock_scheduler();

		void yield_internal();

	protected:
		void suspend_task() override;
		void lock() override;
		void unlock() override;

	public:
		void acquire_task(Task& new_task, threads::priority_t recommended_priority) override;
		Task *get_current_task() override;
		Task *pop_task() override;
		void irq_fired() override;
		void yield() override;
	};
}   // namespace threads::schedulers

#endif   // HUGOS_ROUNDROBIN_NOPRIORITY_PREEMPTIVE_HPP
