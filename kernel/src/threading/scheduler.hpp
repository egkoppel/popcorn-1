
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_SCHEDULER_HPP
#define HUGOS_SCHEDULER_HPP

#include "task.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <utility/handle_table.hpp>

namespace threads {
	using time_t     = uint64_t;
	using priority_t = uint64_t;

	class GlobalScheduler {
	private:
		/**
		 * All tasks that exist
		 */
		SyscallHandleTable<std::unique_ptr<Task>, syscall_handle_type::syscall_handle_type::TASK> task_handles_list;
		std::map<time_t, Task&> sleep_queue;

		static std::unique_ptr<GlobalScheduler> instance;

	public:
		GlobalScheduler() = default;
		static GlobalScheduler& get() { return *instance; }

		/**
		 * @brief Get a task object from a syscall handle
		 * @param task Syscall handle for the task to find
		 * @return Pointer to the found task - will return nullptr if handle is invalid
		 * @note Task pointer is not invalidated while task is running
		 */
		Task *get_task(syscall_handle_t task) {
			return task_handles_list.get_data_from_handle(task, std::unique_ptr<Task>()).get();
		}

		void add_to_sleep_queue(Task& task, time_t handle) { this->sleep_queue.insert({handle, task}); }
		void irq_fired();
		/**
		 * @brief Unblocks the task and causes it to be scheduled, if it is currently blocked
		 * @param task The task to unblock
		 */
		void unblock_task(Task& task);
		time_t current_time();
	};

	class ILocalScheduler {
	protected:
		/**
		 * @brief Called by the currently running task to suspend execution of itself
		 * @remark Current implementations should immediately relinquish control of the task and not schedule until
		 * `acquire_task()` is called on it, however future revisions may change this behaviour, for example to keep
		 * tasks pinned to a specific core
		 */
		virtual void suspend_task() = 0;

		/**
		 * @brief Pair of functions for locking and unlocking the local scheduler, eg. to change the state of a task and
		 * perform another associated action
		 * @attention Between a call to lock() and a call to unlock(), the following actions may not take place:
		 *   - Changes to task state
		 *   - Task switches
		 * Upon calling unlock(), any actions that would have happened, eg. a context switch due to a task blocking,
		 * should immediately take place
		 */
		///@{
		virtual void lock()   = 0;
		virtual void unlock() = 0;
		///@}

	public:
		virtual ~ILocalScheduler() = default;

		/**
		 * @brief Causes the local scheduler to acquire the passed task, and optionally schedule it at the @p
		 * recommended_priority
		 * @note The task to acquire will always be in a `READY_TO_RUN` state
		 * @param new_task The task to begin scheduling
		 * @param recommended_priority Recommended priority to schedule the task at, likely based on its priority before
		 * being last blocked
		 */
		virtual void acquire_task(Task& new_task, priority_t recommended_priority) = 0;

		/**
		 * @brief Called by the currently running task to suspend execution of itself
		 * @param reason The state the task will change to
		 */
		void block_task(Task::State reason);

		/**
		 * @brief Retrieves the currently executing task
		 * @return Pointer to the task currently executing - will return nullptr if scheduler is idle
		 */
		virtual Task *get_current_task() = 0;

		/**
		 * @brief Pops a task off the execution queue, likely used to balance load between schedulers
		 * @return Pointer to the popped task - will return nullptr if no available tasks
		 * @note Should modify scheduler internal state so the task is no longer scheduled by the current scheduler, but
		 * should not adjust the state of the task
		 * @attention Should never return the currently running task
		 */
		virtual Task *pop_task() = 0;

		/**
		 * @brief Called when the scheduling timer fires
		 */
		virtual void irq_fired() = 0;

		/**
		 * @brief Called by the currently running task to relinquish control to other tasks
		 * @note Should not cause the task to become blocked
		 */
		virtual void yield() = 0;

		/**
		 * @brief Called by the currently running task to suspend execution until the global time counter is equal to or
		 * greater than a value
		 * @param time_ns The point at which to resume execution, in nanoseconds
		 */
		void sleep_until(time_t time_ns);
		void sleep_ns(time_t time_ns) { this->sleep_until(GlobalScheduler::get().current_time() + time_ns); }

		void perform_while_locked(auto&& f) {
			this->lock();
			f();
			this->unlock();
		}
	};

	extern cpu_local ILocalScheduler *local_scheduler;
}   // namespace threads

#endif   // HUGOS_SCHEDULER_HPP
