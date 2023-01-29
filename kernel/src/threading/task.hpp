
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

#include <functional>
#include <memory/memory_map.hpp>
#include <memory/paging.hpp>
#include <memory/stack.hpp>
#include <memory/virtual_allocators/monotonic_allocator.hpp>
#include <memory>
#include <stdatomic.h>

namespace threads {
	struct kernel_task_t {};
	struct user_task_t {};
	inline constexpr kernel_task_t kernel_task{};
	inline constexpr user_task_t user_task{};

	class Task {
	public:
		enum class State { RUNNING, SLEEPING, FUTEX, PAUSED, KILLED };

	private:
		struct allocator_wrapper {
			allocator_wrapper(Task *task) : task(task) {}
			memory::aligned<memory::vaddr_t> allocate(std::size_t);
			void deallocate(memory::aligned<memory::vaddr_t>, std::size_t);
			Task *task;
		};

		static atomic_uint_fast64_t next_pid;

		memory::KStack<> stack;
		memory::paging::AddressSpace address_space_;
		memory::vaddr_t stack_ptr_;
		uint64_t pid = atomic_fetch_add(&next_pid, 1);
		State state  = State::RUNNING;
		const char *name_;
		memory::virtual_allocators::MonotonicAllocator allocator;
		std::vector<memory::MemoryMap<void, allocator_wrapper>> mmaps;
		atomic_uint_fast64_t pending_wake_count = 0;

		explicit Task(const char *name, memory::KStack<>&& stack);
		Task(const char *name, usize argument, usize stack_offset);

	public:
		Task()            = delete;
		Task(const Task&) = delete;
		Task(const Task& other, deep_copy_t)
			: stack(other.stack, deep_copy),
			  // TODO: Copy over the allocations
			  allocator(memory::vaddr_t{.address = memory::constants::userspace_end / 2},
		                memory::vaddr_t{.address = memory::constants::userspace_end}) {}
		Task(Task&&) = default;
		~Task()      = default;
		Task(const char *name, void (*entrypoint)(usize), usize argument, kernel_task_t);
		Task(const char *name, void (*entrypoint)(usize), usize argument, user_task_t);

		static std::unique_ptr<Task> initialise(memory::KStack<>&& current_stack);

		inline void set_state(State state) { this->state = state; }
		inline State get_state() const { return this->state; }
		inline memory::paging::AddressSpace& address_space() { return this->address_space_; }
		inline const memory::paging::AddressSpace& address_space() const { return this->address_space_; }
		memory::vaddr_t& stack_ptr() { return this->stack_ptr_; }
		const memory::vaddr_t& stack_ptr() const { return this->stack_ptr_; }
		const memory::KStack<>& kernel_stack() const { return this->stack; }
		memory::vaddr_t new_mmap(memory::vaddr_t hint, usize size, bool downwards);
		const char *name() { return this->name_; }
		void send_signal();
		u64 pending_signals() { return this->pending_wake_count; }
		void decrement_pending_signals() { atomic_fetch_sub(&this->pending_wake_count, 1); }
	};

	extern "C" usize get_p4_table_frame(const Task *);
	extern "C" usize *task_stack_ptr_storage(Task *);
	extern "C" usize get_kstack_top(const Task *);
}   // namespace threads

#endif   // HUGOS_KERNEL_SRC_THREADING_TASK_HPP
