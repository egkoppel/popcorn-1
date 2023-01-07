
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

#include <arch/interrupts.hpp>

void unhandled_irq(arch::interrupt_info_t *info) noexcept {
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

extern "C" void exception_handler_entry(arch::interrupt_info_t *info) noexcept {
	switch (info->vector) {
		case 0x8: interrupt_handlers::double_fault(info); break;
		case 0xE: interrupt_handlers::page_fault(info); break;
		default: unhandled_irq(info); break;
	}
}
