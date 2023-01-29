
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
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
	using time_t     = u64;
	using priority_t = u64;

	class ILocalScheduler;

	class GlobalScheduler {
		friend class ILocalScheduler;

	private:
		std::vector<std::unique_ptr<Task>> tasks;
		/*std::vector<*/ std::reference_wrapper<ILocalScheduler> /*>*/ schedulers
				= std::reference_wrapper<ILocalScheduler>(*static_cast<ILocalScheduler *>(nullptr));
		static std::unique_ptr<GlobalScheduler> instance;

	public:
		GlobalScheduler() = default;

		/**
		 * Get the GlobalScheduler instance
		 */
		static GlobalScheduler& get() {
			if (!instance) instance = std::make_unique<GlobalScheduler>();
			return *instance;
		}

		/**
		 * Add a task to the global list of schedulable tasks
		 * @param task The task to add
		 */
		void add_task(std::unique_ptr<Task> task);

		/**
		 * Add a task to the global list of schedulable tasks
		 * @param task The task to add
		 */
		void make_local_scheduler(std::unique_ptr<Task> current_task);

		// void add_to_sleep_queue(Task& task, time_t handle) {}   // { this->sleep_queue.insert({handle, task}); }

		/**
		 * Called when the global timer finishes counting down
		 */
		void irq_fired();

		/**
		 * @brief Unblocks the task and causes it to be scheduled, if it is currently blocked
		 * @param task The task to unblock
		 */
		void unblock_task(Task& task);

		/**
		 * @brief Gets the current time in nanoseconds
		 *
		 * Returns the global time since an implementation defined point, counting each tick in nanoseconds
		 * @return The time count
		 */
		time_t current_time();
	};

	extern "C" void task_startup_scheduler_unlock();

	class ILocalScheduler {
		friend void ::threads::task_startup_scheduler_unlock();

	protected:
		/**
		 * @brief Called by the currently running task to suspend execution of itself
		 * @remark Current implementations should immediately relinquish control of the task and not schedule until
		 * `acquire_task()` is called on it again, however future revisions may change this behaviour, for example to
		 * keep tasks pinned to a specific core
		 * @see acquire_task()
		 */
		virtual void suspend_task() = 0;

		virtual void lock_structures() =0;
		virtual void unlock_structures() =0;

	public:
		///@{
		/**
		 * @brief Pair of functions for locking and unlocking the local scheduler, eg. to change the state of a task and
		 * perform another associated action
		 * @attention Between a call to lock() and a call to unlock(), the following actions may not take place:
		 *   - Changes to task state
		 *   - Task switches
		 *
		 * Upon calling unlock(), any actions that would have happened, eg. a context switch due to a task blocking,
		 * should immediately take place
		 */
		virtual void lock()   = 0;
		virtual void unlock() = 0;
		///@}

	public:
		static ILocalScheduler& create_local_scheduler(Task *currently_running_task = nullptr);

		virtual ~ILocalScheduler() = default;

		/**
		 * @brief Adds the task to the scheduler
		 *
		 * Adds the task to the scheduler's internal task list, and causes it to be a task that is able to be scheduled
		 * by the scheduler
		 * @param new_task The task to begin scheduling
		 * @param recommended_priority Recommended priority to schedule the task at
		 * @note The task to acquire will always be in a `READY_TO_RUN` state
		 */
		virtual void acquire_task(Task& new_task, priority_t recommended_priority) = 0;

		/**
		 * @brief Suspends execution of the currently running task
		 * @param reason The reason the task is blocked
		 * @warning \p reason should not be `READY_TO_RUN`
		 */
		void block_task(Task::State reason);

		/**
		 * @brief Gets the currently executing task
		 * @return Pointer to the task currently executing - returns `nullptr` if currently idle
		 */
		virtual Task *get_current_task() = 0;

		/**
		 * @brief Pops a task off the execution queue, likely used to balance load between schedulers
		 * @return Pointer to the popped task - returns `nullptr` if no available tasks
		 * @note Should modify scheduler internal state so the task is no longer scheduled by the current scheduler, but
		 * should not adjust the state of the task
		 * @remark Implementations should normally try to ensure the returned task is the task with the longest time
		 * until being executed
		 * @attention Should never return the currently running task
		 */
		virtual Task *pop_task() = 0;

		/**
		 * @brief Called when the local scheduler timer fires at its fixed frequency
		 */
		virtual void irq_fired() = 0;

		/**
		 * @brief Yields execution of the currently running task
		 * @note Should not cause the task to become blocked
		 */
		virtual void yield() = 0;

		/**
		 * @brief Suspends execution of the current task until the global time reaches the requested time
		 * @param time_ns The point at which to resume execution, in nanoseconds
		 * @see GlobalScheduler::current_time()
		 */
		void sleep_until(time_t time_ns);

		/**
		 * @brief Suspends execution for the requested amount of time
		 * @param time_ns The amount of time to suspend execution for, in nanoseconds
		 */
		void sleep_ns(time_t time_ns) { this->sleep_until(GlobalScheduler::get().current_time() + time_ns); }
	};

	/**
	 * The scheduler for the current CPU
	 */
	extern cpu_local std::unique_ptr<ILocalScheduler> local_scheduler;
}   // namespace threads

#endif   // HUGOS_SCHEDULER_HPP
