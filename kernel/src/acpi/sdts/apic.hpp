
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_APIC_HPP
#define HUGOS_APIC_HPP

#include "sdt.hpp"

#include <memory/types.hpp>
#include <utils.h>

namespace acpi {
	namespace madt {
		enum class entry_type : u8 {
			CPU_LAPIC                        = 0,
			IO_APIC                          = 1,
			IOAPIC_INTERRUPT_SOURCE_OVERRIDE = 2,
			IOAPIC_NMI_SOURCE                = 3,
			LAPIC_NMI                        = 4,
			LAPIC_ADDR                       = 5,
			LAPIC_X2                         = 9
		};

		struct [[gnu::packed]] entry_t {
			entry_type type;
			u8 length;
		};

		struct [[gnu::packed]] lapic_entry_t : entry_t {
			u8 cpu_id;
			u8 apic_id;
			u32 flags;
		};

		struct [[gnu::packed]] ioapic_entry_t : entry_t {
			u8 apic_id;
			u8 _reserved;
			memory::paddr32_t ioapic_addr;
			u32 global_system_interrupt_base;
		};

		struct [[gnu::packed]] ioapic_source_override_entry_t : entry_t {
			u8 bus_source;
			u8 irq_source;
			u32 gsi;
			u16 flags;
		};

		struct [[gnu::packed]] lapic_addr_entry_t : entry_t {
			memory::paddr_t addr;
		};
	}   // namespace madt

	struct [[gnu::packed]] madt_t : system_description_table_t {
	public:
		class iterator {
		public:
			explicit iterator(madt::entry_t *entry) : entry(entry) {}

			friend std::strong_ordering operator<=>(const iterator&, const iterator&) = default;
			friend bool operator!=(const iterator&, const iterator&)                  = default;
			iterator& operator++() {
				this->entry = reinterpret_cast<madt::entry_t *>(ADD_BYTES(this->entry, this->entry->length));
				return *this;
			}
			madt::entry_t& operator*() { return *this->entry; }

		private:
			madt::entry_t *entry;
		};

		memory::paddr_t lapic_addr() const noexcept { return (this->lapic_address); }
		iterator begin() noexcept { return iterator(&this->entries[0]); }
		iterator end() noexcept { return iterator(reinterpret_cast<madt::entry_t *>(ADD_BYTES(this, this->length))); }

	private:
		memory::paddr32_t lapic_address;
		u32 flags;
		madt::entry_t entries[];
	};
}   // namespace acpi

#endif   // HUGOS_APIC_HPP
