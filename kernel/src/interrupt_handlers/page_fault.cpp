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

[[noreturn]] void interrupt_handlers::page_fault(arch::interrupt_info_t *interrupt_info) noexcept {
	fprintf(stdserial, "Page fault!\n");
	fprintf(stdserial, "Error code: %d\n", interrupt_info->error_code);
	fprintf(stdserial, "IP: %lp\n", interrupt_info->ip);
	fprintf(stdserial, "Flags: 0x%08x\n", interrupt_info->flags);
	fprintf(stdserial, "SP: %lp\n", interrupt_info->sp);
	fprintf(stdserial, "Attempted access to: %lp\n", interrupt_info->page_fault_memory_addr);

	while (true) hal::nop();
}
