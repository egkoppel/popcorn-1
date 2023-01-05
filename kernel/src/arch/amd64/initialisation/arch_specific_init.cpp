/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../interrupts/idt.hpp"
#include "../interrupts/pic.hpp"
#include "../interrupts/syscall.hpp"
#include "gdt.hpp"
#include "tss.hpp"

#include <arch/initialisation.hpp>

using namespace arch::amd64;

namespace arch {
	namespace {
		using interrupt_handler_t = void (*)() noexcept;
		extern "C" interrupt_handler_t isr_stub_table[];

		void idt_init() {
			for (std::size_t i = 0; i < 256; ++i) { interrupt_descriptor_table.add_entry(i, 0, isr_stub_table[i]); }
		}
	}   // namespace

	int arch_specific_early_init() {
		global_descriptor_table.add_entry(GDT::Entry::new_code(0, true), GDT::EntryType::KERNEL_CODE);
		global_descriptor_table.add_entry(GDT::Entry::new_data(0, true), GDT::EntryType::KERNEL_DATA);
		global_descriptor_table.add_entry(GDT::Entry::new_code(3, true), GDT::EntryType::USER_CODE_LONG_MODE);
		global_descriptor_table.add_entry(GDT::Entry::new_data(3, true), GDT::EntryType::USER_DATA);
		global_descriptor_table.add_system_entry(GDT::SystemEntry::new_tss(&task_state_segment),
		                                         GDT::SystemEntryType::TSS);
		global_descriptor_table.load();

		idt_init();
		interrupt_descriptor_table.load();

		syscall_register_init();

		return 0;
	}

	int arch_specific_late_init() {
		return -1;
	}
}   // namespace arch