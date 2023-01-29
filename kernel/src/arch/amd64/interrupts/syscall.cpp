/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "syscall.hpp"

#include <arch/amd64/initialisation/gdt.hpp>
#include <arch/amd64/macros.hpp>
#include <arch/interrupts.hpp>

namespace arch {
	namespace {
		syscall_handler_t handler;
	}

	namespace amd64 {
		namespace {
			// Called as   rax(*)(rax, rdi, rsi, rdx, r8, r9)
			// System V is rax(*)(rdi, rsi, rdx, rcx, r8, r9)
			[[gnu::naked, clang::no_sanitize("undefined")]] u64 syscall_long_mode_entry() {
				__asm__ volatile("movq %rsp, %r12");   // save userspace stack ptr
				__asm__ volatile("movq %0, %%rsp"
				                 :
				                 : "m"(task_state_segment.privilege_stack_table[0].address));   // load kernel stack
				__asm__ volatile("pushq %r12");                                                 // stores stack pointer
				__asm__ volatile("pushq %rcx");                                                 // stores rip
				__asm__ volatile("pushq %r11");                                                 // stores rflags
				__asm__ volatile(
						"mov %rdx, %rcx;"   // See register mappings above
						"mov %rsi, %rdx;"
						"mov %rdi, %rsi;"
						"mov %rax, %rdi;");
				__asm__ volatile("call *%0" : : "m"(handler));
				__asm__ volatile("popq %r11");
				__asm__ volatile("popq %rcx");
				__asm__ volatile("popq %rsp");
				__asm__ volatile("sysretq");
			}

			/*[[gnu::naked]] int64_t syscall_compat_mode_entry() {

		}*/

			enum SfmaskBits { IF = 1 << 9 };

			constexpr u32 SFMASK = 0xC0000084;
			constexpr u32 STAR   = 0xC0000081;
			constexpr u32 LSTAR  = 0xC0000082;
		}   // namespace

		void syscall_register_init() noexcept {
			// Set IF bit in SFMASK so interrupts disabled on syscall entry
			wrsmr(SFMASK, SfmaskBits::IF);
			wrsmr(LSTAR, reinterpret_cast<uint64_t>(syscall_long_mode_entry));
			wrsmr(STAR,
			      (static_cast<u64>(GDT::EntryType::USER_CODE_COMPAT_MODE) * 8 | 0b11) << 48
			              | static_cast<u64>(GDT::EntryType::KERNEL_CODE) * 8 << 32);
		}
	}   // namespace amd64

	void load_syscall_handler(syscall_handler_t handler_func) noexcept {
		handler = handler_func;
	}
}   // namespace arch
