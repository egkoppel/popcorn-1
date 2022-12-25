
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_THREADING_TASK_HPP
#define HUGOS_KERNEL_SRC_THREADING_TASK_HPP

#include <memory/paging.hpp>
#include <memory/stack.hpp>
#include <memory>
#include <stdatomic.h>

namespace threads {
	class Task {
	public:
		enum class State { RUNNING, SLEEPING, FUTEX, PAUSED };

	private:
		static atomic_uint_fast64_t next_pid;

		memory::KStack<> stack;
		memory::paging::AddressSpace page_table;
		memory::vaddr_t stack_ptr;
		uint64_t pid = atomic_fetch_add(&next_pid, 1);
		State state  = State::RUNNING;

		Task(memory::IPhysicalAllocator& stack_frame_allocator, memory::IPhysicalAllocator& page_table_allocator); /* :
			stack{2 * memory::constants::frame_size, stack_frame_allocator},
			page_table{page_table_allocator} {}*/

	public:
		static std::unique_ptr<Task> new_task(memory::IPhysicalAllocator& stack_frame_allocator,
		                                      memory::IPhysicalAllocator& page_table_allocator) {
			return std::unique_ptr(new Task(stack_frame_allocator, page_table_allocator));
		}

		static std::unique_ptr<Task> init_multitasking(memory::KStack<>&& current_stack) {
			return std::unique_ptr<Task>(nullptr);
		}

		inline void set_state(State state) { this->state = state; }
		inline State get_state() const { return this->state; }
		inline memory::paging::AddressSpace& address_space() { return this->page_table; }
		inline const memory::paging::AddressSpace& address_space() const { return this->page_table; }
	};
}   // namespace threads

#endif   //HUGOS_KERNEL_SRC_THREADING_TASK_HPP
