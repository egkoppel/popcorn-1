
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_GDT_HPP
#define HUGOS_GDT_HPP

#include "tss.hpp"

#include <cstdint>

namespace arch::amd64 {
	class [[gnu::packed]] alignas(8) GDT {
	public:
		class SystemEntry;

		class Entry {
			friend class arch::amd64::GDT::SystemEntry;

		private:
			u64 data = 0;
			explicit Entry(u64 data) noexcept : data(data) {}
			Entry(u64 addr, u64 limit, u64 access_byte, u64 db, u64 granularity, u64 long_mode) noexcept {
				this->data = 0;
				this->data |= limit & 0xFFFF;
				this->data |= (addr & 0xFFFF) << 16;
				this->data |= ((addr >> 16) & 0xFF) << 32;
				this->data |= (access_byte & 0xFF) << 40;
				this->data |= ((limit >> 16) & 0xF) << 48;
				this->data |= (long_mode & 0x1) << 53;
				this->data |= (db & 0x1) << 54;
				this->data |= (granularity & 0x1) << 55;
				this->data |= ((addr >> 24) & 0xFF) << 56;
			}

			static Entry new_tss_low(memory::vaddr_t address, u64 size, u64 dpl) noexcept;

		public:
			Entry() noexcept = default;
			static Entry new_code(u64 dpl, bool long_mode) noexcept;
			static Entry new_data(u64 dpl, bool long_mode) noexcept;
		};

		class SystemEntry {
		private:
			u64 data_low  = 0;
			u64 data_high = 0;
			SystemEntry(Entry low, u64 high) noexcept : data_low(low.data), data_high(high) {}

		public:
			Entry low() const noexcept { return Entry(this->data_low); }
			Entry high() const noexcept { return Entry(this->data_high); }
			static SystemEntry new_tss(TSS *tss_addr) noexcept;
		};

	private:
		Entry entries[8];

	public:
		enum class EntryType {
			NULL_SEGMENT          = 0,
			KERNEL_CODE           = 1,
			KERNEL_DATA           = 2,
			USER_CODE_COMPAT_MODE = 3,
			USER_DATA             = 4,
			USER_CODE_LONG_MODE   = 5
		};

		enum class SystemEntryType { TSS = 6 };

		uint8_t add_entry(Entry entry, EntryType type) noexcept;
		uint8_t add_system_entry(SystemEntry entry, SystemEntryType type) noexcept;
		void load() noexcept;
	};

	extern GDT global_descriptor_table;
}   // namespace arch::amd64

#endif   //HUGOS_GDT_HPP
