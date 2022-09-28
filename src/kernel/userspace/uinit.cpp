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
#include "dyld.hpp"

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
	syscall(syscall_vectors::print, (uint64_t)"\033c");
	syscall(syscall_vectors::print, (uint64_t)hugOS_ascii_logo);
	syscall(syscall_vectors::print, (uint64_t)"Welcome to userspace\n\n");
	*reinterpret_cast<volatile int *>(NULL) = 5;

	while (1);
}
