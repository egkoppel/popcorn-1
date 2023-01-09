
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gdt.hpp"

#include <popcorn_prelude.h>
#include <stdint.h>

namespace arch::amd64 {
	struct [[gnu::packed]] gdt_ptr {
		u16 size;
		u64 address;
	};

	u8 GDT::add_entry(GDT::Entry entry, EntryType type) noexcept {
		this->entries[static_cast<u8>(type)] = entry;
		return static_cast<u8>(type);
	}

	u8 GDT::add_system_entry(GDT::SystemEntry entry, SystemEntryType type) noexcept {
		this->entries[static_cast<u8>(type)]     = entry.low();
		this->entries[static_cast<u8>(type) + 1] = entry.high();
		return static_cast<u8>(type);
	}

	void GDT::load() noexcept {
		gdt_ptr ptr = {.size = sizeof(GDT::entries) - 1, .address = reinterpret_cast<u64>(this)};

		__asm__ volatile("lgdt %0" : : "m"(ptr));
		__asm__ volatile("pushq $0x8; leaq .1$(%%rip), %%rax; pushq %%rax; lretq; .1$:" : : : "rax");
	}

	GDT::Entry GDT::Entry::new_code(u64 dpl, bool long_mode) noexcept {
		u8 access_byte = 0x98;
		access_byte |= (dpl & 0b11) << 5;
		return {0, 0xFFFFF, access_byte, 0, 1, long_mode};
	}

	GDT::Entry GDT::Entry::new_data(u64 dpl, bool long_mode) noexcept {
		uint8_t access_byte = 0x92;
		access_byte |= (dpl & 0b11) << 5;
		return {0, 0xFFFFF, access_byte, 0, 1, long_mode};
	}

	GDT::Entry GDT::Entry::new_tss_low(memory::vaddr_t address, u64 size, u64 dpl) noexcept {
		u8 access_byte = 0x89;
		access_byte |= (dpl & 0b11) << 5;
		return {address.address, size, access_byte, 0, 1, 1};
	}

	GDT::SystemEntry GDT::SystemEntry::new_tss(TSS *tss_addr) noexcept {
		return {GDT::Entry::new_tss_low(memory::vaddr_t{.address = reinterpret_cast<usize>(tss_addr)},
		                                sizeof(*tss_addr),
		                                0),
		        reinterpret_cast<u64>(tss_addr) >> 32};
	}

	GDT global_descriptor_table = GDT();
}   // namespace arch::amd64
