
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
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
		memory::paging::AddressSpace address_space_;
		memory::vaddr_t stack_ptr_;
		uint64_t pid = atomic_fetch_add(&next_pid, 1);
		State state  = State::RUNNING;
		const char *name;

		explicit Task(const char *name, memory::KStack<>&& stack);

	public:
		Task()            = delete;
		Task(const Task&) = delete;
		Task(const Task& other, deep_copy_t) : stack(other.stack, deep_copy) {}
		Task(Task&&) = default;
		~Task()      = default;
		Task(const char *name, void (*entrypoint)(usize), usize argument);

		static std::unique_ptr<Task> initialise(memory::KStack<>&& current_stack);

		inline void set_state(State state) { this->state = state; }
		inline State get_state() const { return this->state; }
		inline memory::paging::AddressSpace& address_space() { return this->address_space_; }
		inline const memory::paging::AddressSpace& address_space() const { return this->address_space_; }
		memory::vaddr_t& stack_ptr() { return this->stack_ptr_; }
		const memory::vaddr_t& stack_ptr() const { return this->stack_ptr_; }
		const memory::KStack<>& kernel_stack() const { return this->stack; }
	};

	extern "C" usize get_p4_table_frame(const Task *);
	extern "C" usize *task_stack_ptr_storage(Task *);
	extern "C" usize get_kstack_top(const Task *);
}   // namespace threads

#endif   // HUGOS_KERNEL_SRC_THREADING_TASK_HPP
