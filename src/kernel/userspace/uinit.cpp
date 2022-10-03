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

extern "C" int uinit() {
	journal_log("uinit started\n");
	journal_log("[TARGET REACHED] Pre-fsd\n");
	void *fsd_online_sem = sem_init(1);
	sys_spawn("fsd", fsd_main, fsd_online_sem);
	sem_wait(fsd_online_sem);
	journal_log("[TARGET REACHED] fsd\n");

	while (1);
}
