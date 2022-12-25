
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gdt.hpp"

#include <stdint.h>

namespace arch::amd64 {

	struct [[gnu::packed]] gdt_ptr {
		uint16_t size;
		uint64_t address;
	};

	uint8_t GDT::add_entry(GDT::Entry entry, EntryType type) noexcept {
		this->entries[static_cast<uint8_t>(type)] = entry;
		return static_cast<uint8_t>(type);
	}

	uint8_t GDT::add_system_entry(GDT::SystemEntry entry, SystemEntryType type) noexcept {
		this->entries[static_cast<uint8_t>(type)]     = entry.low();
		this->entries[static_cast<uint8_t>(type) + 1] = entry.high();
		return static_cast<uint8_t>(type);
	}

	void GDT::load() noexcept {
		gdt_ptr ptr = {.size = sizeof(GDT::entries) - 1, .address = reinterpret_cast<uint64_t>(this)};

		__asm__ volatile("lgdt %0" : : "m"(ptr));
		__asm__ volatile("pushq $0x8; leaq .1$(%%rip), %%rax; pushq %%rax; lretq; .1$:" : : : "rax");
	}

	GDT::Entry GDT::Entry::new_code(uint64_t dpl, bool long_mode) noexcept {
		uint8_t access_byte = 0x98;
		access_byte |= (dpl & 0b11) << 5;
		return {0, 0xFFFFF, access_byte, 0, 1, long_mode};
	}

	GDT::Entry GDT::Entry::new_data(uint64_t dpl, bool long_mode) noexcept {
		uint8_t access_byte = 0x92;
		access_byte |= (dpl & 0b11) << 5;
		return {0, 0xFFFFF, access_byte, 0, 1, long_mode};
	}

	GDT::Entry GDT::Entry::new_tss_low(memory::vaddr_t address, uint64_t size, uint64_t dpl) noexcept {
		uint8_t access_byte = 0x89;
		access_byte |= (dpl & 0b11) << 5;
		return {address.address, size, access_byte, 0, 1, 1};
	}

	GDT::SystemEntry GDT::SystemEntry::new_tss(TSS *tss_addr) noexcept {
		return {GDT::Entry::new_tss_low(memory::vaddr_t{.address = reinterpret_cast<usize>(tss_addr)},
		                                sizeof(*tss_addr),
		                                0),
		        reinterpret_cast<uint64_t>(tss_addr) >> 32};
	}

	GDT global_descriptor_table = GDT();
}   // namespace arch::amd64