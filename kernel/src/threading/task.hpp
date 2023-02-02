
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

#include <cstddef>
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
		const char *name;
		memory::virtual_allocators::MonotonicAllocator allocator;
		std::vector<memory::MemoryMap<std::byte, allocator_wrapper>> mmaps;

		explicit Task(const char *name, memory::KStack<>&& stack);
		Task(const char *name, const usize (&args)[6], usize stack_offset);

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
		template<class... Args> requires(sizeof...(Args) <= 6)
		[[clang::no_sanitize("pointer-overflow")]] Task(const char *name,
		                                                void (*entrypoint)(Args...) noexcept,
		                                                kernel_task_t,
		                                                Args&&...args)
			: Task(name, {std::bit_cast<usize>(args)...}, 0) {
			auto stack_top = static_cast<u64 *>((*this->stack.top()).address);
			stack_top[-1]  = reinterpret_cast<u64>(entrypoint);
		}

		template<class... Args> requires(sizeof...(Args) <= 6)
		[[clang::no_sanitize("pointer-overflow")]] Task(const char *name,
		                                                void (*entrypoint)(Args...) noexcept,
		                                                user_task_t,
		                                                Args&&...args)
			: Task(name, {std::bit_cast<usize>(args)...}, 1) {
			auto stack_top = static_cast<u64 *>((*this->stack.top()).address);
			stack_top[-1]  = reinterpret_cast<u64>(entrypoint);
			stack_top[-2]  = reinterpret_cast<u64>(arch::switch_to_user_mode);
		}

		static std::unique_ptr<Task> initialise(memory::KStack<>&& current_stack);

		inline void set_state(State state) { this->state = state; }
		inline State get_state() const { return this->state; }
		inline memory::paging::AddressSpace& address_space() { return this->address_space_; }
		inline const memory::paging::AddressSpace& address_space() const { return this->address_space_; }
		memory::vaddr_t& stack_ptr() { return this->stack_ptr_; }
		const memory::vaddr_t& stack_ptr() const { return this->stack_ptr_; }
		const memory::KStack<>& kernel_stack() const { return this->stack; }
		memory::vaddr_t new_mmap(memory::vaddr_t hint, usize size, bool downwards);
	};

	extern "C" usize get_p4_table_frame(const Task *);
	extern "C" usize *task_stack_ptr_storage(Task *);
	extern "C" usize get_kstack_top(const Task *);
}   // namespace threads

#endif   // HUGOS_KERNEL_SRC_THREADING_TASK_HPP
