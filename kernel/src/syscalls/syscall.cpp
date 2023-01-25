
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

#include "ipc.hpp"

#include <bit>

namespace syscalls {
	int64_t syscall_entry(SyscallVectors syscall_number,
	                      int64_t arg1,
	                      int64_t arg2,
	                      int64_t arg3,
	                      int64_t arg4,
	                      int64_t arg5) noexcept {
		switch (syscall_number) {
			case SyscallVectors::get_current_task: break;
			case SyscallVectors::yield: break;
			case SyscallVectors::exit: break;
			case SyscallVectors::sleep: break;
			case SyscallVectors::suspend: break;
			case SyscallVectors::resume: break;
			case SyscallVectors::spawn: break;
			case SyscallVectors::spawn2: break;
			case SyscallVectors::get_time_used: break;
			case SyscallVectors::mailbox_new: break;
			case SyscallVectors::mailbox_send: break;
			case SyscallVectors::mailbox_recv: break;
			case SyscallVectors::mailbox_destroy: break;
			case SyscallVectors::mailbox_transfer: break;
			case SyscallVectors::mailbox_reply: break;
			case SyscallVectors::mailbox_send_with_reply: break;
			case SyscallVectors::region_new: break;
			case SyscallVectors::region_new_anon: break;
			case SyscallVectors::region_new_dma: break;
			case SyscallVectors::set_flags: break;
			case SyscallVectors::map_region: break;
			case SyscallVectors::share_region: break;
			case SyscallVectors::mutex_lock: break;
			case SyscallVectors::mutex_try_lock: break;
			case SyscallVectors::mutex_unlock: break;
			case SyscallVectors::mutex_new: break;
			case SyscallVectors::mutex_destroy: break;
			case SyscallVectors::sem_post: break;
			case SyscallVectors::sem_wait: break;
			case SyscallVectors::sem_get_count: break;
			case SyscallVectors::sem_new: break;
			case SyscallVectors::sem_destroy: break;
			case SyscallVectors::ipc_register: break;
			case SyscallVectors::ipc_open: break;
			case SyscallVectors::ipc_close: break;
			case SyscallVectors::ipc_wait: break;
			case SyscallVectors::ipc_notify: break;
		}
		return INT64_MIN;
	}
}   // namespace syscalls
