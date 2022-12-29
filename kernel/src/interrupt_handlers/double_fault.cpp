
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "double_fault.hpp"

#include <arch/hal.hpp>

[[noreturn]] void interrupt_handlers::double_fault(arch::interrupt_info_t *interrupt_info) noexcept {
	LOG(Log::CRITICAL,
	    "Double fault!\n"
	    "Error code %d\n"
	    "IP: %lp\n"
	    "Flags: 0x%08x\n"
	    "SP: %lp",
	    interrupt_info->error_code,
	    interrupt_info->ip,
	    interrupt_info->flags,
	    interrupt_info->sp);

	/*if (interrupt_info->page_fault_memory_addr >= reinterpret_cast<uint64_t>(&level4_page_table) &&
	    cr2 < reinterpret_cast<uint64_t>(&level4_page_table) + 0x1000) {
		fprintf(stdserial, "CR2: %lp - possible stack overflow\n", cr2);
		panic("Potential stack overflow");
	}*/

	//trace_stack_trace(10, old_base_ptr);

	while (true) hal::nop();
}
