
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_THREADING_FUTEX_HPP
#define HUGOS_KERNEL_SRC_THREADING_FUTEX_HPP

#include "task.hpp"

#include <deque>
#include <utility/lock_free_queue.hpp>

namespace threads {
	class Futex {
	private:
		structures::lock_free::queue<Task *> waiting_tasks;

	public:
		/**
		 * If `*addr == val_to_check`, blocks current thread, otherwise returns immediately
		 * @param addr Futex value location
		 * @param val_to_check
		 */
		void wait(atomic_size_t *addr, size_t val_to_check);
		void notify(const size_t num_to_wake);
	};
}   // namespace threads

#endif   //HUGOS_KERNEL_SRC_THREADING_FUTEX_HPP
