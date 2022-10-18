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

[[noreturn]] int message_test(int64_t mailbox) {
	char buf[256] = {5, 1, 4, 7, 1, 6, 8, 2, 4, 7, 8, 2, 1};
	uint64_t status = send_msg(mailbox, 0, buf);

	while (true) __asm__ volatile("");
}

[[noreturn]] int uinit_main(void *ramfs_data, uint64_t ramfs_size) {
	void *fsd_online_sem = sem_init(1);
	sys_spawn_3("fsd", fsd_start, fsd_online_sem, ramfs_data, ramfs_size);
	sem_wait(fsd_online_sem);

	auto mbox_handle = mbox_new();
	sys_spawn_1("test", message_test, mbox_handle);
	threads::message_t buf;
	recv_msg(mbox_handle, 0, &buf);

	mbox_destroy(mbox_handle);
	yield();

	while (true) __asm__ volatile("");
}

[[noreturn, gnu::naked]] int uinit() {
	__asm__ volatile("pop %rsi"); // pop argument off stack
	__asm__ volatile("pop %rdi"); // pop argument off stack
	__asm__ volatile("andq $-16, %rsp"); // align stack
	__asm__ volatile("jmp %P0" : : "i"(uinit_main));
}
