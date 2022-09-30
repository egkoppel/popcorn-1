
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_APIC_HPP
#define HUG_APIC_HPP

#include "acpi.hpp"

enum class madt_entry_types : uint8_t {
	CPU_LAPIC = 0,
	IO_APIC = 1,
	IOAPIC_INTERRUPT_SOURCE_OVERRIDE = 2,
	IOAPIC_NMI_SOURCE = 3,
	LAPIC_NMI = 4,
	LAPIC_ADDR = 5,
	LAPIC_X2 = 9
};

extern "C" struct madt_entry_header {
	madt_entry_types type;
	uint8_t length;
} __attribute__((packed));

extern "C" struct madt_entry_lapic {
	madt_entry_header header;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__((packed));

extern "C" struct madt_entry_ioapic {
	madt_entry_header header;
	uint8_t apic_id;
private:
	uint8_t _reserved;
public:
	uint32_t io_apic_addr;
	uint32_t global_system_interrupt_base;
} __attribute__((packed));

extern "C" struct madt_entry_lapic_addr {
	madt_entry_header header;
	uint64_t lapic_addr;
} __attribute__((packed));

struct madt_entry_header_iterator {
private:
	madt_entry_header *i;
public:
	explicit madt_entry_header_iterator(madt_entry_header *i) : i(i) {}
	void operator ++() {
		i = (madt_entry_header *)((uintptr_t)i + i->length);
	}
	madt_entry_header *operator ->() {
		return i;
	}
	madt_entry_header& operator *() {
		return *i;
	}
	bool operator ==(const madt_entry_header_iterator& rhs) const = default;
	bool operator !=(const madt_entry_header_iterator& rhs) const = default;
};

extern "C" struct MADT {
	AcpiSdtHeader header;

	uint32_t lapic_address;
	uint32_t flags;
	madt_entry_header first_entry;

	madt_entry_header_iterator begin() {
		return madt_entry_header_iterator(&this->first_entry);
	}
	madt_entry_header_iterator end() {
		return madt_entry_header_iterator((madt_entry_header *)((uintptr_t)this + this->header.length));
	}
} __attribute__((packed));

#endif //HUG_APIC_HPP
