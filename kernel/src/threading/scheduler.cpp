/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "scheduler.hpp"

namespace threads {

	void ILocalScheduler::block_task(Task::State reason) {
		this->get_current_task()->set_state(
				reason);   // Can safely assume get_current_task is valid pointer as block can only be called by current task
		this->suspend_task();
	}

	void ILocalScheduler::sleep_until(time_t time_ns) {
		decltype(auto) global_scheduler = GlobalScheduler::get();
		global_scheduler.add_to_sleep_queue(
				*this->get_current_task(),
				time_ns);   // Can safely assume get_current_task is valid pointer as sleep can only be called by current task
		this->block_task(Task::State::SLEEPING);
	}

	std::unique_ptr<GlobalScheduler> GlobalScheduler::instance = nullptr;

	void GlobalScheduler::irq_fired() {}

	cpu_local ILocalScheduler *local_scheduler;
}   // namespace threads