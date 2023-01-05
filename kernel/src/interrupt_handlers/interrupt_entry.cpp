
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

extern "C" void exception_handler_entry(arch::interrupt_info_t *info) noexcept {
	switch (info->vector) {
		case 0x8: interrupt_handlers::double_fault(info); break;
		case 0xE: interrupt_handlers::page_fault(info); break;
		default: LOG(Log::INFO, "Unhandled irq at vector 0x%llx", info->vector); break;
	}
}
