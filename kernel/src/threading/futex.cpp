
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "futex.hpp"

#include "scheduler.hpp"

#include <stdatomic.h>
/*
 * here for explanation of what happens with futexes

static void mutex_unlock(lock) {
	if (atomic_fetch_sub(lock, 1) == 1) return;   // returns old value: 1 -> no tasks waiting
	else {
		lock = 0; // mark as unlocked
		futex_wake(lock, num_tasks: 1); // wait first waiting task
	}
}

static void mutex_lock(lock) {
	size_t val;
	if ((val = atomic_compare_exchange_strong(lock, expected: 0, change_to: 1)) == 0) return;   // returns old value: 0 -> nothing had lock
	else { // something else has lock
		if (val != 2) val = atomic_exchange(lock, change_to: 2); // mark as waiting task
		while (val != 0) {
			futex_wait(lock, return_if_not_still: 2); // if nothing unlocked in between, go to sleep
			val = atomic_exchange(lock, change_to: 2); // if woken up, mark as locked/waiting
                                                       // exits if fully unlocked before we locked it
                                                       // otherwise marks as waiting and goes round loop again
		}
	}
}
*/

void threads::Futex::wait(atomic_size_t *addr, size_t val_to_check) {
	auto& sched = *local_scheduler;

	this->waiting_tasks.push(sched.get_current_task());

	// TODO ENSURE(SMP): Add paired memory barrier here to ensure that any threads will update the futex before we read it
	// I *think* the load with memory_order_acquire should be good for this?

	// If wake() was called and value changed, don't block
	// Don't need to pop task off queue as notify() will do that unconditionally
	if (atomic_load_explicit(addr, memory_order_acquire) == val_to_check) sched.block_task(Task::State::FUTEX);
}

void threads::Futex::notify(const size_t num_to_wake) {
	// TODO ENSURE(SMP): Add paired memory barrier here to ensure that this thread updates any futex values before read by wait()
	// I *think* the atomic_thread_fence should be good for this?
	atomic_thread_fence(memory_order_release);

	// Unblock first n waiting tasks unconditionally
	for (size_t i = 0; i < num_to_wake; ++i) {
		Task *task_to_unblock;
		if (this->waiting_tasks.pop(&task_to_unblock)) { GlobalScheduler::get().unblock_task(*task_to_unblock); }
	}
}
