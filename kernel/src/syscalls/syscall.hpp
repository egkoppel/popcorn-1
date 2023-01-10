
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_KERNEL_SRC_SYSCALLS_SYSCALL_HPP
#define HUGOS_KERNEL_SRC_SYSCALLS_SYSCALL_HPP

#include <stdint.h>

enum class SyscallVectors : uint64_t {
	get_current_task = 0x000, /* syscall_handle_t get_current_task() */
	yield            = 0x001, /* int64_t yield() */
	exit             = 0x002, /* void exit() */
	sleep            = 0x003, /* int64_t sleep(uint64_t) */
	suspend          = 0x004, /* int64_t suspend() */
	resume           = 0x005, /* int64_t resume(syscall_handle_t) */
	spawn = 0x006, /* syscall_handle_t spawn(char*, void(*)(uint64_t, uint64_t, uint64_t), uint64_t, uint64_t, uint64_t) */
	spawn2        = 0x007, /* syscall_handle_t spawn(char*, char**, char**) */
	get_time_used = 0x008, /* int64_t get_time_used() */

	mailbox_new             = 0x200,
	mailbox_send            = 0x201,
	mailbox_recv            = 0x202,
	mailbox_destroy         = 0x203,
	mailbox_transfer        = 0x204,
	mailbox_reply           = 0x205,
	mailbox_send_with_reply = 0x206,

	region_new      = 0x300,
	region_new_anon = 0x301,
	region_new_dma  = 0x302,
	set_flags       = 0x303,
	map_region      = 0x304,
	share_region    = 0x306,

	mutex_lock,
	mutex_try_lock,
	mutex_unlock,
	mutex_new,
	mutex_destroy,

	sem_post,
	sem_wait,
	sem_get_count,
	sem_new,
	sem_destroy
};

int64_t syscall_entry(SyscallVectors syscall_number,
                      int64_t arg1,
                      int64_t arg2,
                      int64_t arg3,
                      int64_t arg4,
                      int64_t arg5) noexcept;

#endif   //HUGOS_KERNEL_SRC_SYSCALLS_SYSCALL_HPP
