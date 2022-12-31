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
	}   // namespace

#define GS_SWAP

#define SYSCALL_ENTRY_ASM                                                                                              \
	GS_SWAP                                                                                                            \
	__asm__ volatile("pushq %rax");                                                                                    \
	__asm__ volatile("pushq %rcx");                                                                                    \
	__asm__ volatile("pushq %rdx");                                                                                    \
	__asm__ volatile("pushq %rsi");                                                                                    \
	__asm__ volatile("pushq %rdi");                                                                                    \
	__asm__ volatile("pushq %r8");                                                                                     \
	__asm__ volatile("pushq %r9");                                                                                     \
	__asm__ volatile("pushq %r10");                                                                                    \
	__asm__ volatile("pushq %r11");                                                                                    \
	__asm__ volatile("movq %rsp, %rdi");                                                                               \
	__asm__ volatile("addq $72, %rdi");                                                                                \
	__asm__ volatile("movq %rbp, %rsi");

#define SYSCALL_EXIT_ASM                                                                                               \
	__asm__ volatile("popq %r11");                                                                                     \
	__asm__ volatile("popq %r10");                                                                                     \
	__asm__ volatile("popq %r9");                                                                                      \
	__asm__ volatile("popq %r8");                                                                                      \
	__asm__ volatile("popq %rdi");                                                                                     \
	__asm__ volatile("popq %rsi");                                                                                     \
	__asm__ volatile("popq %rdx");                                                                                     \
	__asm__ volatile("popq %rcx");                                                                                     \
	__asm__ volatile("popq %rax");                                                                                     \
	GS_SWAP                                                                                                            \
	__asm__ volatile("iretq");

#define SYSCALL_EXIT_ASM_ERRCODE                                                                                       \
	__asm__ volatile("popq %r11");                                                                                     \
	__asm__ volatile("popq %r10");                                                                                     \
	__asm__ volatile("popq %r9");                                                                                      \
	__asm__ volatile("popq %r8");                                                                                      \
	__asm__ volatile("popq %rdi");                                                                                     \
	__asm__ volatile("popq %rsi");                                                                                     \
	__asm__ volatile("popq %rdx");                                                                                     \
	__asm__ volatile("popq %rcx");                                                                                     \
	__asm__ volatile("popq %rax");                                                                                     \
	__asm__ volatile("add $8, %rsp");                                                                                  \
	GS_SWAP                                                                                                            \
	__asm__ volatile("iretq");

#define NO_RETURN_NO_ERROR_CODE(name, body)                                                                            \
	void name##_inner(exception_stack_frame_t *frame, uint64_t old_base_ptr) { body }                                  \
	extern "C" __attribute__((naked)) void name() {                                                                    \
		__asm__ volatile("movq %rsp, %rdi");                                                                           \
		__asm__ volatile("movq %rbp, %rsi");                                                                           \
		__asm__ volatile("call %P0" : : "i"(name##_inner));                                                            \
		__asm__ volatile("cli; hlt;");                                                                                 \
	}

#define NO_RETURN_ERROR_CODE(name, body)                                                                               \
	void name##_inner(exception_stack_frame_error_t *frame, uint64_t old_base_ptr) { body }                            \
	extern "C" __attribute__((naked)) void name() {                                                                    \
		__asm__ volatile("movq %rsp, %rdi");                                                                           \
		__asm__ volatile("movq %rbp, %rsi");                                                                           \
		__asm__ volatile("call %P0" : : "i"(name##_inner));                                                            \
		__asm__ volatile("cli; hlt;");                                                                                 \
	}

#define RETURN_ERROR_CODE(name, body)                                                                                  \
	void name##_inner(exception_stack_frame_error_t *frame, uint64_t old_base_ptr) { body }                            \
	extern "C" __attribute__((naked)) void name() {                                                                    \
		SYSCALL_ENTRY_ASM                                                                                              \
		__asm__ volatile("call %P0" : : "i"(name##_inner));                                                            \
		SYSCALL_EXIT_ASM_ERRCODE                                                                                       \
	}

#define RETURN_NO_ERROR_CODE(name, body)                                                                               \
	void name##_inner(exception_stack_frame_t *frame, uint64_t old_base_ptr) { body }                                  \
	extern "C" __attribute__((naked)) void name() {                                                                    \
		SYSCALL_ENTRY_ASM                                                                                              \
		__asm__ volatile("call %P0" : : "i"(name##_inner));                                                            \
		SYSCALL_EXIT_ASM                                                                                               \
	}

	struct __attribute__((packed)) exception_stack_frame_error_t {
		u64 error_code;
		u64 ip;
		u64 cs;
		u64 flags;
		u64 sp;
		u64 ss;
	};

	struct __attribute__((packed)) exception_stack_frame_t {
		u64 ip;
		u64 cs;
		u64 flags;
		u64 sp;
		u64 ss;
	};

	interrupt_handler_t handlers[48];

	RETURN_ERROR_CODE(page_fault, {
		({
			interrupt_info_t info{.error_code             = frame->error_code,
			                      .ip                     = frame->ip,
			                      .sp                     = frame->sp,
			                      .page_fault_memory_addr = 0,
			                      .flags                  = frame->flags};
			__asm__ volatile("mov %%cr2, %0" : "=r"(info.page_fault_memory_addr));
			(handlers[vector_to_idt_index(InterruptVectors::PAGE_FAULT)])(&info);
		});
	})

	NO_RETURN_ERROR_CODE(double_fault, {
		({
			interrupt_info_t info{.error_code             = frame->error_code,
			                      .ip                     = frame->ip,
			                      .sp                     = frame->sp,
			                      .page_fault_memory_addr = 0,
			                      .flags                  = frame->flags};
			(handlers[vector_to_idt_index(InterruptVectors::DOUBLE_FAULT)])(&info);
		});
	})

	void load_interrupt_handler(InterruptVectors vector,
	                            bool user_callable,
	                            uint8_t stack_idx,
	                            interrupt_handler_t handler) {
		handlers[vector_to_idt_index(vector)] = handler;

		void (*handler_wrapper)() = nullptr;

		switch (vector) {
			case InterruptVectors::PAGE_FAULT: handler_wrapper = page_fault; break;
			case InterruptVectors::DOUBLE_FAULT: handler_wrapper = double_fault; break;
			case InterruptVectors::CORE_TIMER: [[fallthrough]];
			case InterruptVectors::GLOBAL_TIMER: [[fallthrough]];
			case InterruptVectors::LAST: return;
		}

		amd64::interrupt_descriptor_table.add_entry(vector_to_idt_index(vector),
		                                            user_callable ? 3 : 0,
		                                            handler_wrapper,
		                                            stack_idx);
	}

	memory::KStack<> backup_stacks[8];

	void load_backup_stack(uint8_t stack_idx, memory::KStack<>&& stack) {
		if (stack_idx >= 8) throw std::runtime_error("Only 7 backup stacks supported");
		backup_stacks[stack_idx] = std::move(stack);
		amd64::task_state_segment.add_stack(stack_idx, backup_stacks[stack_idx]);
	}
}   // namespace arch