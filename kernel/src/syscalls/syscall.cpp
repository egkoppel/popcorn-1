
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "syscall.hpp"

#include "irq.hpp"

#include <limits>
#include <new>
#include <threading/scheduler.hpp>

extern "C" int64_t syscall_entry(SyscallVectors syscall_number,
                                 int64_t arg1,
                                 int64_t arg2,
                                 int64_t arg3,
                                 int64_t arg4,
                                 int64_t arg5) noexcept try {
	switch (syscall_number) {
		case SyscallVectors::get_current_task: return -1;
		case SyscallVectors::yield: return -1;
		case SyscallVectors::exit: return -1;
		case SyscallVectors::sleep: return -1;
		case SyscallVectors::suspend: return -1;
		case SyscallVectors::resume: return -1;
		case SyscallVectors::spawn: return -1;
		case SyscallVectors::spawn2: return -1;
		case SyscallVectors::make_stack: {
			auto stack_top = threads::local_scheduler->get_current_task()->new_mmap(memory::vaddr_t{.address = 0},
			                                                                        4096,
			                                                                        true);
			return std::bit_cast<i64>(stack_top.address);
		}
		case SyscallVectors::get_time_used: return -1;
		case SyscallVectors::mailbox_new: return -1;
		case SyscallVectors::mailbox_send: return -1;
		case SyscallVectors::mailbox_recv: return -1;
		case SyscallVectors::mailbox_destroy: return -1;
		case SyscallVectors::mailbox_transfer: return -1;
		case SyscallVectors::mailbox_reply: return -1;
		case SyscallVectors::mailbox_send_with_reply: return -1;
		case SyscallVectors::region_new: return -1;
		case SyscallVectors::region_new_anon: return -1;
		case SyscallVectors::region_new_dma: return -1;
		case SyscallVectors::set_flags: return -1;
		case SyscallVectors::map_region: return -1;
		case SyscallVectors::share_region: return -1;
		case SyscallVectors::register_isa_irq: return syscall::register_isa_irq(arg1);
		case SyscallVectors::unregister_isa_irq: return syscall::unregister_isa_irq(arg1);
		case SyscallVectors::mutex_lock: return -1;
		case SyscallVectors::mutex_try_lock: return -1;
		case SyscallVectors::mutex_unlock: return -1;
		case SyscallVectors::mutex_new: return -1;
		case SyscallVectors::mutex_destroy: return -1;
		case SyscallVectors::sem_post: return -1;
		case SyscallVectors::sem_wait: return -1;
		case SyscallVectors::sem_get_count: return -1;
		case SyscallVectors::sem_new: return -1;
		case SyscallVectors::sem_destroy: return -1;
		default: return -2;
	}
} catch (std::bad_alloc&) { return -3; } catch (...) {
	return std::numeric_limits<i64>::min();
}
