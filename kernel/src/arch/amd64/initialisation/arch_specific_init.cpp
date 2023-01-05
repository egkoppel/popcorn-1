/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../interrupts/syscall.hpp"
#include "gdt.hpp"
#include "tss.hpp"
#include "../interrupts/idt.hpp"

#include <arch/initialisation.hpp>

using namespace arch::amd64;

namespace arch {
	int arch_specific_early_init() {
		global_descriptor_table.add_entry(GDT::Entry::new_code(0, true), GDT::EntryType::KERNEL_CODE);
		global_descriptor_table.add_entry(GDT::Entry::new_data(0, true), GDT::EntryType::KERNEL_DATA);
		global_descriptor_table.add_entry(GDT::Entry::new_code(3, true), GDT::EntryType::USER_CODE_LONG_MODE);
		global_descriptor_table.add_entry(GDT::Entry::new_data(3, true), GDT::EntryType::USER_DATA);
		global_descriptor_table.add_system_entry(GDT::SystemEntry::new_tss(&task_state_segment),
		                                         GDT::SystemEntryType::TSS);
		global_descriptor_table.load();

		interrupt_descriptor_table.load();

		syscall_register_init();

		return 0;
	}

	int arch_specific_late_init() {
		return -1;
	}
}   // namespace arch