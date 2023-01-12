
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "double_fault.hpp"
#include "page_fault.hpp"

#include <amd64_macros.hpp>
#include <arch/interrupts.hpp>
#include <main/debug.hpp>
#include <smp/cpu.hpp>
#include <syscalls/irq.hpp>
#include <threading/scheduler.hpp>
#include <threading/task.hpp>

void unhandled_irq(arch::interrupt_info_t *info) noexcept {
	bool handled = false;
	for (auto&& irq : irq_list) {
		if (irq.first == info->vector) {
			handled = true;
			threads::GlobalScheduler::get().unblock_task(irq.second.get());
		}
	}
	if (true /* TODO: check if lapic irq */) Cpu::lapic->eoi();
	if (handled) return;

	usize rsp;
	__asm__ volatile("mov %%rsp, %0" : "=r"(rsp));
	LOG(Log::WARNING,
	    "Unhandled irq at vector 0x%llx\n"
	    "Error code %d\n"
	    "IP: %lp\n"
	    "Flags: 0x%08x\n"
	    "SP: %lp\n"
	    "Attempted access to: %lp\n"
	    "New stack pointer: %lp",
	    info->vector,
	    info->error_code,
	    info->ip,
	    info->flags,
	    info->sp,
	    info->page_fault_memory_addr,
	    rsp);
}

[[noreturn]] void general_protection_fault(arch::interrupt_info_t *interrupt_info) noexcept {
	LOG(Log::CRITICAL,
	    "General protection fault!\n"
	    "Error code %d\n"
	    "IP: %lp\n"
	    "Flags: 0x%08x\n"
	    "SP: %lp",
	    interrupt_info->error_code,
	    interrupt_info->ip,
	    interrupt_info->flags,
	    interrupt_info->sp);
	while (true) hlt();
}

extern "C" void exception_handler_entry(arch::interrupt_info_t *info, usize rbp) noexcept {
	// trace_stack_trace(100, rbp);
	switch (info->vector) {
		case 0xd: general_protection_fault(info); break;
		case 0x8: interrupt_handlers::double_fault(info); break;
		case 0xE: interrupt_handlers::page_fault(info); break;
		default: unhandled_irq(info); break;
	}
}
