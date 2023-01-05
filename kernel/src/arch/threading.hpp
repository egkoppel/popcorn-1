
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_THREADING_HPP
#define HUGOS_THREADING_HPP

#include <threading/task.hpp>

namespace arch {
	extern "C" void task_startup();
	extern "C" void task_switch_asm(threads::Task *new_task, threads::Task *old_task);
	extern "C" void switch_to_user_mode();
}   // namespace arch

#endif   // HUGOS_THREADING_HPP
