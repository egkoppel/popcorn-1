/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mutex.hpp"
#include "../smp/core_local.hpp"

using namespace threads;

std::shared_ptr<Task> Mutex::get_next_waiting_task() {
	if (!this->waiting_tasks.empty()) {
		auto task = this->waiting_tasks.front();
		this->waiting_tasks.pop_front();
		return task;
	}
	return nullptr;
}

void Mutex::lock() {
	get_local_data()->scheduler.lock_task_switches();

	if (this->locked) {
		this->add_waiting_task(get_local_data()->scheduler.current_task_ptr);
		get_local_data()->scheduler.block_task(task_state::WAITING_FOR_LOCK);
	} else { this->locked = true; }

	get_local_data()->scheduler.unlock_task_switches();
}

uint64_t Mutex::try_lock() {
	get_local_data()->scheduler.lock_task_switches();

	if (this->locked) { return 1; }
	else { this->locked = true; }

	get_local_data()->scheduler.unlock_task_switches();
	return 0;
}

void Mutex::unlock() {
	get_local_data()->scheduler.lock_task_switches();

	if (auto task_to_wake = this->get_next_waiting_task()) {
		get_local_data()->scheduler.unblock_task(task_to_wake);
	} else { this->locked = false; }

	get_local_data()->scheduler.unlock_task_switches();
}