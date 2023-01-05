
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "task.hpp"

#include <arch/threading.hpp>

using namespace memory;

namespace threads {
	atomic_uint_fast64_t Task::next_pid = 1;

	Task::Task(const char *name, memory::KStack<>&& stack)
		: stack(std::move(stack)),
		  address_space_(),
		  stack_ptr_(0_va),
		  name(name) {}

	std::unique_ptr<Task> Task::initialise(memory::KStack<>&& current_stack) {
		auto ktask = new Task{"kmain", std::move(current_stack)};
		return std::unique_ptr<Task>(ktask);
	}

	Task::Task(const char *name, void (*entrypoint)(usize), usize argument)
		: stack(memory::constants::frame_size),
		  address_space_(),
		  stack_ptr_(vaddr_t(*this->stack.top()) - 8 * 8),
		  name(name) {
		auto stack_top = static_cast<u64 *>((*this->stack.top()).address);
		stack_top[-1]  = reinterpret_cast<u64>(entrypoint);
		stack_top[-2]  = reinterpret_cast<u64>(arch::task_startup);
		stack_top[-3]  = argument;
		stack_top[-4]  = 0;
		stack_top[-5]  = 0;
		stack_top[-6]  = 0;
		stack_top[-7]  = 0;
		stack_top[-8]  = 0;
	}

	usize get_p4_table_frame(const Task *task) {
		auto ret = task->address_space().l4_table_frame()->addr();
		LOG(Log::DEBUG, "get_p4_table_frame() -> %lp", ret);
		return ret;
	}
	usize *task_stack_ptr_storage(Task *task) {
		return &task->stack_ptr().address;
	}
	usize get_kstack_top(const Task *task) {
		return (*task->kernel_stack().top()).address.address;
	}
}   // namespace threads
