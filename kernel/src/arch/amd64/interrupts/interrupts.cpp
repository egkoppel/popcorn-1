/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../initialisation/tss.hpp"
#include "idt.hpp"

#include <arch/interrupts.hpp>

namespace arch {
	namespace {
		namespace PIC_IRQ {
			enum PIC_IRQ {
				PIC1        = 0x20,
				TIMER       = PIC1,
				KEYBOARD    = PIC1 + 1,
				SERIAL2     = PIC1 + 3,
				SERIAL1     = PIC1 + 4,
				PARALLEL2_3 = PIC1 + 5,
				FD          = PIC1 + 6,
				PARALLEL1   = PIC1 + 7,

				PIC2  = 0x28,
				RTC   = PIC2,
				ACPI  = PIC2 + 1,
				MOUSE = PIC2 + 4,
				X87   = PIC2 + 5,
				ATA_P = PIC2 + 6,
				ATA_S = PIC2 + 7
			};
		}

		constexpr uint8_t vector_to_idt_index(InterruptVectors vector) noexcept {
			/**
			 * 0x00 - divide error (no error code)
			 * 0x01 - debug exception (no error code)
			 * 0x02 - non-maskable interrupt (no error code)
			 * 0x03 - breakpoint (no error code)
			 * 0x04 - overflow (no error code)
			 * 0x05 - bound range exceeded (no error code)
			 * 0x06 - invalid opcode (no error code)
			 * 0x07 - device not available (no error code)
			 * 0x08 - double fault (error code)
			 * 0x09 - coprocessor segment overrun (no error code)
			 * 0x0A - invalid TSS (error code)
			 * 0x0B - segment not present (error code)
			 * 0x0C - stack-segment fault (error code)
			 * 0x0D - general protection fault (error code)
			 * 0x0E - page fault (error code)
			 * 0x0F - reserved (no error code)
			 */
			switch (vector) {
				case InterruptVectors::PAGE_FAULT: return 0xE;
				case InterruptVectors::CORE_TIMER: return PIC_IRQ::TIMER;
				case InterruptVectors::GLOBAL_TIMER: return 255;
				case InterruptVectors::LAST: return 255;
				case InterruptVectors::DOUBLE_FAULT: return 0x8;
			}
		}
	};   // namespace

	void set_interrupt_perms(u8 vector, bool user_callable, uint8_t stack_idx) {
		auto idt_idx = vector;   // vector_to_idt_index(vector);
		amd64::interrupt_descriptor_table.set_flags(idt_idx, user_callable ? 3 : 0, stack_idx);
	}

	memory::KStack<> backup_stacks[8];

	void load_backup_stack(uint8_t stack_idx, memory::KStack<>&& stack) {
		if (stack_idx >= 8) throw std::runtime_error("Only 7 backup stacks supported");
		backup_stacks[stack_idx] = std::move(stack);
		amd64::task_state_segment.add_stack(stack_idx, backup_stacks[stack_idx]);
	}
}   // namespace arch