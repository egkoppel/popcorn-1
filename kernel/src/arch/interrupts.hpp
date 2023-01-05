
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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

	struct interrupt_info_t {
		uint64_t error_code;
		uint64_t ip;
		uint64_t sp;
		uint64_t page_fault_memory_addr;
		uint64_t flags;
	};

	using syscall_handler_t   = int64_t (*)(SyscallVectors, int64_t, int64_t, int64_t, int64_t, int64_t) noexcept;
	using interrupt_handler_t = void (*)(interrupt_info_t *) noexcept;

	void load_syscall_handler(syscall_handler_t) noexcept;
	void load_interrupt_handler(InterruptVectors vector, bool user_callable, uint8_t stack_idx, interrupt_handler_t);
	void load_backup_stack(uint8_t stack_idx, memory::KStack<>&& stack);
}   // namespace arch

#endif   // HUGOS_INTERRUPTS_HPP
