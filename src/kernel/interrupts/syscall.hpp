/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HUGOS_SYSCALL_H
#define _HUGOS_SYSCALL_H

#include <stdint.h>

extern "C" void syscall_long_mode_handler();

namespace syscall_vectors {
	enum syscall_vectors : uint64_t {
		yield, /* void yeild() */
		get_time_used, /* uint64_t get_time_used() */
		sleep,

		mutex_lock,
		mutex_try_lock,
		mutex_unlock,
		mutex_new,
		mutex_destroy,

		sem_post,
		sem_wait,
		sem_get_count,
		sem_new,
		sem_destroy,

		spawn,
		mmap_anon,
		munmap_anon,

		journal_log,

		get_pid_by_name,

		send_msg,
		wait_msg,
		get_msg
	};
}

namespace mmap_flags {
	enum mmap_flags {
		PROT_NONE = 1ull << 0,
		PROT_READ = 1ull << 1,
		PROT_WRITE = 1ull << 2,
		PROT_EXEC = 1ull << 3,

		MAP_FILE = 1ull << 4,
		MAP_PRIVATE = 1ull << 5,
		MAP_SHARED = 1ull << 6,
		MAP_FIXED = 1ull << 7,
		MAP_ANON = 1ull << 8,
		MAP_ANONYMOUS = 1ull << 8,
		MAP_NORESERVE = 1ull << 9
	};
}

uint64_t syscall_handler(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif
