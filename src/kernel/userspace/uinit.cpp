/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define HUGOS_USERSPACE

#include "uinit.hpp"
#include "initramfs.hpp"
#include "../interrupts/syscall.hpp"
#include <stdint.h>
#include <stdarg.h>
#include "userspace_macros.hpp"
#include "fsd.hpp"

#define hugOS_ascii_logo \
" \n\
 _                  ____   _____ \n\
| |                / __ \\ / ____| \n\
| |__  _   _  __ _| |  | | (___ \n\
| '_ \\| | | |/ _` | |  | |\\___ \\ \n\
| | | | |_| | (_| | |__| |____) | \n\
|_| |_|\\__,_|\\__, |\\____/|_____/ \n\
              __/ | \n\
             |___/ \n\
"

[[noreturn]] int uinit_main(uint64_t ramfs_data, uint64_t ramfs_size) {
	void *fsd_online_sem = sem_init(1);
	auto fsd_mbox = mbox_new();
	auto fsd_task_handle = sys_spawn_2("fsd", fsd_start, fsd_online_sem, fsd_mbox);
	mbox_transfer(fsd_mbox, fsd_task_handle);

	sem_wait(fsd_online_sem);

	fsd_command_t mount_ramfs_command = {
			.command = fsd_command_t::MOUNT,
			.data = {
					.mount = {
							.driver_command_len = 0,
							.mountpoint = 'A',
							.driver_info = "initramfs \0"
					}
			}
	};
	char ramfs_data_str[32];
	char ramfs_size_str[32];
	utoa(ramfs_data, ramfs_data_str, 10);
	utoa(ramfs_size, ramfs_size_str, 10);
	strcat(mount_ramfs_command.data.mount.driver_info, ramfs_data_str);
	strcat(mount_ramfs_command.data.mount.driver_info, " ");
	strcat(mount_ramfs_command.data.mount.driver_info, ramfs_size_str);
	mount_ramfs_command.data.mount.driver_command_len = strlen(mount_ramfs_command.data.mount.driver_info);

	send_msg_with_reply(fsd_mbox, UINT64_MAX, &mount_ramfs_command);
	
	auto mount_command_response = reinterpret_cast<fsd_command_response_t *>(&mount_ramfs_command);

	while (true) __asm__ volatile("");
}

[[noreturn, gnu::naked]] int uinit() {
	__asm__ volatile("pop %rsi"); // pop argument off stack
	__asm__ volatile("pop %rdi"); // pop argument off stack
	__asm__ volatile("andq $-16, %rsp"); // align stack
	__asm__ volatile("jmp %P0" : : "i"(uinit_main));
}
