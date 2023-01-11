
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_ACPI_IOAPIC_HPP
#define POPCORN_KERNEL_SRC_ACPI_IOAPIC_HPP

#include <memory/memory_map.hpp>
#include <memory/types.hpp>
#include <optional>
#include <popcorn_prelude.h>
#include <utility/bitfield.hpp>
#include <vector>

namespace acpi {
	struct redirection_entry;

	namespace detail {
		struct ioapic_registers;

		class ioapic_register {
		public:
			ioapic_register(u32 num, ioapic_registers *);
			ioapic_register(volatile ioapic_register&& other) : ioapic(other.ioapic), num(other.num) {}

			explicit(false) operator u32() volatile;
			volatile ioapic_register& operator=(u32) volatile;
			volatile ioapic_register& operator&=(u32) volatile;
			volatile ioapic_register& operator|=(u32) volatile;

		private:
			ioapic_registers *ioapic;
			u32 num;
		};

		class ioapic_register_64 {
		public:
			ioapic_register_64(u32 num, ioapic_registers *);
			ioapic_register_64(volatile ioapic_register_64&& other)
				: ioapic(other.ioapic),
				  num_lower(other.num_lower) {}

			explicit(false) operator u64() volatile;
			volatile ioapic_register_64& operator=(u64) volatile;
			volatile ioapic_register_64& operator&=(u64) volatile;
			volatile ioapic_register_64& operator|=(u64) volatile;
			volatile ioapic_register_64& operator^=(u64) volatile;

		private:
			ioapic_registers *ioapic;
			u32 num_lower;
		};

		class ioapic_redirection_registers {
		public:
			ioapic_redirection_registers(ioapic_registers *);

			redirection_entry operator[](usize irq);

		private:
			ioapic_registers *ioapic;
		};

		struct ioapic_registers {
			ioapic_register version() { return {1, this}; }
			ioapic_redirection_registers redirections() { return {this}; }

			volatile u32 *reg_sel() { return reinterpret_cast<volatile u32 *>(this); }
			volatile u32 *reg_win() { return reinterpret_cast<volatile u32 *>(this) + 4; }
		};
	}   // namespace detail

	struct redirection_entry {
		volatile detail::ioapic_register_64 reg;

		bitfield_member<decltype(redirection_entry::reg), 0, 0xFF, u64> vector() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 8, 0x7, u64> delivery_mode() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 11, 1, u64> destination_mode() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 13, 1, u64> active_low() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 15, 1, u64> level_triggered() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 16, 1, u64> mask() { return {this->reg}; }
		bitfield_member<decltype(redirection_entry::reg), 56, 0xFF, u64> destination() { return {this->reg}; }
	};

	struct ioapic_t {
		static constexpr auto flags = memory::paging::PageTableFlags::WRITEABLE
		                              | memory::paging::PageTableFlags::NO_EXECUTE
		                              | memory::paging::PageTableFlags::IMPL_CACHE_DISABLE
		                              | memory::paging::PageTableFlags::IMPL_CACHE_WRITETHROUGH
		                              | memory::paging::PageTableFlags::GLOBAL;

		ioapic_t(memory::paddr_t ioapic_base, u32 gsi_start, memory::IPhysicalAllocator& allocator);

		auto& operator->() { return this->registers; }

		memory::MemoryMap<detail::ioapic_registers> registers;
		u32 gsi_start;
		u32 gsi_end;
	};

	class Ioapics {
	public:
		void add_ioapic(ioapic_t&& ioapic);
		void add_iso(u32 irq, u32 gsi, u32 flags);
		redirection_entry redirection_entry(u32 gsi);
		u32 pic_irq_to_gsi(u32 irq);

	private:
		std::vector<ioapic_t> ioapics;
		using bad_dict = std::vector<std::pair<u32, u32>>;
		bad_dict isos;

		std::optional<ioapic_t *> find_ioapic_for_gsi(u32 gsi);
	};
}   // namespace acpi

#endif   // POPCORN_KERNEL_SRC_ACPI_IOAPIC_HPP
