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

enum class syscall_vectors : uint64_t {
	yield, /* void yeild() */
	serial_write, /* int serial_write(char* str) */
	print,
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

	message_send, /* int message_send(uint64_t receiver_pid, uint64_t message) */
	mailbox_create,

	new_proc,
	mmap_anon
};

namespace mmap_prot {
	enum mmap_prot {
		PROT_EXEC = 1 << 0,
		PROT_READ = 1 << 1,
		PROT_WRITE = 1 << 2,
		PROT_NONE = 1 << 3
	};
}

uint64_t syscall_handler(syscall_vectors syscall_number, uint64_t arg1, uint64_t arg2);
inline uint64_t __attribute__((naked)) syscall(syscall_vectors syscall_number, uint64_t arg1 = 0, uint64_t arg2 = 0) {
	__asm__ volatile("syscall; ret;" :);
}

#endif
