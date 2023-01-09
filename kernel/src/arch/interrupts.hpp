
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_INTERRUPTS_HPP
#define HUGOS_INTERRUPTS_HPP

#include <memory/stack.hpp>
#include <popcorn_prelude.h>
#include <stdint.h>
#include <syscalls/syscall.hpp>

namespace arch {
	enum class InterruptVectors {
		PAGE_FAULT,
		CORE_TIMER,
		GLOBAL_TIMER,
		DOUBLE_FAULT,

		LAST
	};

#ifdef __amd64__
	struct interrupt_info_t {
		u64 page_fault_memory_addr;
		u64 vector;
		u64 error_code;
		u64 ip;
		u64 _cs;
		u64 flags;
		u64 sp;
		u64 _ss;
	};
#endif

	using syscall_handler_t   = i64 (*)(SyscallVectors, i64, i64, i64, int64_t, i64) noexcept;
	using interrupt_handler_t = void (*)(interrupt_info_t *) noexcept;

	void load_syscall_handler(syscall_handler_t) noexcept;
	void set_interrupt_perms(u8 vector, bool user_callable, uint8_t stack_idx);
	void load_backup_stack(u8 stack_idx, memory::KStack<>&& stack);
}   // namespace arch

#endif   // HUGOS_INTERRUPTS_HPP
