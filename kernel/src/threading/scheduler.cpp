/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "scheduler.hpp"

#include "schedulers/roundrobin_nopriority_preemptive.hpp"

namespace threads {
	std::unique_ptr<GlobalScheduler> GlobalScheduler::instance = nullptr;

	void GlobalScheduler::irq_fired() {}

	void GlobalScheduler::add_task(std::unique_ptr<Task> task) {
		this->tasks.push_back(std::move(task));
		this->schedulers.get().acquire_task(*this->tasks.back(), 0);
	}

	void GlobalScheduler::unblock_task(Task& task) {
		if (task.get_state() == Task::State::RUNNING) LOG(Log::DEBUG, "Waking already awake task");
		task.set_state(Task::State::RUNNING);
		this->schedulers.get().acquire_task(*this->tasks.back(), 0);
	}

	void GlobalScheduler::make_local_scheduler(std::unique_ptr<Task> current_task) {
		this->schedulers = ILocalScheduler::create_local_scheduler(current_task.get());
		this->tasks.push_back(std::move(current_task));
	}

	ILocalScheduler& ILocalScheduler::create_local_scheduler(Task *currently_running_task) {
		// TODO: some fancy cpuid thing to decide which scheduler to use
		local_scheduler = std::make_unique<schedulers::RoundRobinNoPriorityPreemptive>(currently_running_task);
		return *local_scheduler;
	}

	void ILocalScheduler::block_task(Task::State reason) {
		if (this->get_current_task()->pending_signals() > 0) {
			this->get_current_task()->decrement_pending_signals();
			return;
		}
		this->get_current_task()->set_state(reason);   // Can safely assume get_current_task is valid pointer as block
		                                               // can only be called by current task
		this->suspend_task();
	}

	/*void ILocalScheduler::sleep_until(time_t time_ns) {
	    decltype(auto) global_scheduler = GlobalScheduler::get();
	    global_scheduler.add_to_sleep_queue(*this->get_current_task(), time_ns);   // Can safely assume get_current_task
	                                                                               // is valid pointer as sleep can only
	                                                                               // be called by current task
	    this->block_task(Task::State::SLEEPING);
	}*/

	cpu_local std::unique_ptr<ILocalScheduler> local_scheduler;

	void task_startup_scheduler_unlock() {
		local_scheduler->unlock_structures();
	}
}   // namespace threads
