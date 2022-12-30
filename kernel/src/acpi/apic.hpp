
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_APIC_HPP
#define HUGOS_APIC_HPP

#include "acpi.hpp"

namespace acpi {
	enum class madt_entry_types : uint8_t {
		CPU_LAPIC                        = 0,
		IO_APIC                          = 1,
		IOAPIC_INTERRUPT_SOURCE_OVERRIDE = 2,
		IOAPIC_NMI_SOURCE                = 3,
		LAPIC_NMI                        = 4,
		LAPIC_ADDR                       = 5,
		LAPIC_X2                         = 9
	};

	extern "C" struct [[gnu::packed]] madt_entry_header {
		madt_entry_types type;
		uint8_t length;
	};

	extern "C" struct [[gnu::packed]] madt_entry_lapic {
		madt_entry_header header;
		uint8_t processor_id;
		uint8_t apic_id;
		uint32_t flags;
	};

	extern "C" struct [[gnu::packed]] madt_entry_ioapic {
		madt_entry_header header;
		uint8_t apic_id;

	private:
		[[maybe_unused]] uint8_t _reserved;

	public:
		uint32_t io_apic_addr;
		uint32_t global_system_interrupt_base;
	};

	extern "C" struct [[gnu::packed]] madt_entry_lapic_addr {
		madt_entry_header header;
		memory::paddr_t lapic_addr;
	};

	/*class [[gnu::packed]] Madt : public SystemDescriptionTable {
	public:
		class Entry {
		protected:
			madt_entry_types type_;
			uint8_t length;

		public:
			uint8_t size() const { return this->length; }
			madt_entry_types type() const { return this->type_; }
		};

		class EntryIterator {
		private:
			const Entry *current;

		public:
			explicit EntryIterator(const Entry *start) : current(start){};
			const Entry& operator*() const { return *current; }
			const Entry *operator->() const { return current; }
			bool operator==(const EntryIterator& rhs) const = default;
			bool operator!=(const EntryIterator& rhs) const = default;
			EntryIterator& operator++() {
				this->current = ADD_BYTES(this->current, this->current->size());
				return *this;
			}
		};

	private:
		memory::paddr32_t lapic_address;
		uint32_t flags;
		Entry first_entry;

	public:
		memory::paddr_t lapic() const noexcept { return (this->lapic_address); }
		EntryIterator begin() const noexcept { return EntryIterator(&this->first_entry); }
		EntryIterator end() const noexcept {
			return EntryIterator(reinterpret_cast<const Entry *>(ADD_BYTES(this, this->length)));
		}
	};*/
}   // namespace acpi

#endif   //HUGOS_APIC_HPP
