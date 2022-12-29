/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "page_fault.hpp"

#include <arch/hal.hpp>

void interrupt_handlers::page_fault(arch::interrupt_info_t *interrupt_info) noexcept {
	LOG(Log::INFO,
	    "Page fault!\n"
	    "Error code %d\n"
	    "IP: %lp\n"
	    "Flags: 0x%08x\n"
	    "SP: %lp\n"
	    "Attempted access to: %lp",
	    interrupt_info->error_code,
	    interrupt_info->ip,
	    interrupt_info->flags,
	    interrupt_info->sp,
	    interrupt_info->page_fault_memory_addr);

	if ((interrupt_info->flags & 1 << 0) == 0) {
		// Caused by non-present page
		// Check for CoW/lazy alloc
		if (interrupt_info->page_fault_memory_addr >= memory::constants::mem_map_start
		    && interrupt_info->page_fault_memory_addr < memory::constants::mem_map_end) {
			// Within mem_map region => lazy alloc
			LOG(Log::DEBUG, "Allocating new mem_map area to cover %lp", interrupt_info->page_fault_memory_addr);
			auto addr =
					memory::aligned<memory::vaddr_t>::aligned_down({.address = interrupt_info->page_fault_memory_addr});
			auto frame = allocators.general().allocate();
			auto flags = memory::paging::PageTableFlags::WRITEABLE | memory::paging::PageTableFlags::NO_EXECUTE
			             | memory::paging::PageTableFlags::GLOBAL;
			memory::paging::kas.map_page_to(addr, frame, flags);
			return;
		}
	}

	LOG(Log::CRITICAL, "Unable to resolve page fault");

	while (true) hal::nop();
}
