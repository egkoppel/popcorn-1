
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "task.hpp"

namespace threads {
	atomic_uint_fast64_t Task::next_pid = 1;

	Task::Task(memory::KStack<>&& stack) : stack(std::move(stack)), address_space_(), stack_ptr(0_va) {}

	std::unique_ptr<Task> Task::initialise(memory::KStack<>&& current_stack) {
		return std::unique_ptr<Task>(new Task{std::move(current_stack)});
	}
}   // namespace threads
