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

extern "C" [[noreturn]] int uinit() {
	journal_log("uinit started\n");
	journal_log("[TARGET REACHED] Pre-fsd\n");
	void *fsd_online_sem = sem_init(1);
	sys_spawn_1("fsd", fsd_start, (uint64_t)fsd_online_sem);
	sem_wait(fsd_online_sem);
	journal_log("[TARGET REACHED] fsd\n");

	volatile uint64_t fsd_pid = get_pid_by_name("fsd");
	yield();

	threads::message_t send_buf;
	send_buf._[0] = 5;
	send_buf._[1] = 8;
	send_buf._[7] = 2;
	send_msg(fsd_pid, &send_buf);

	while (true) __asm__ volatile("");
}
