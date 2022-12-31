/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *  
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_ACPI_HPP
#define HUGOS_ACPI_HPP

#include "../memory/memory_map.hpp"

#include <map>
#include <optional>
#include <smp/cpu.hpp>
#include <stdint.h>
#include <string.h>
#include <string>

namespace acpi {
	struct [[gnu::packed]] system_description_table_t {
		char signature[4];
		uint32_t length;
		uint8_t revision;
		uint8_t checksum;
		char oem_id[6];
		char oem_table_id[8];
		uint32_t oem_revision;
		uint32_t creator_id;
		uint32_t creator_revision;

		bool has_signature(const char *s) const noexcept { return !strncmp(s, this->signature, 4); }
	};

	struct [[gnu::packed]] rsdt_t : system_description_table_t {
		class iterator {
		public:
			explicit iterator(const rsdt_t& rsdt, usize idx) : rsdt(rsdt), idx(idx) {
				this->is_xsdt = this->rsdt.is_xsdt();
			}

			memory::paddr_t operator*() {
				if (this->is_xsdt) return this->rsdt.SDT_ptrs.xsdt[this->idx];
				else return this->rsdt.SDT_ptrs.rsdt[this->idx];
			}
			iterator& operator++() {
				++this->idx;
				return *this;
			}

			friend bool operator==(const rsdt_t::iterator& lhs, const rsdt_t::iterator& rhs) {
				return (&lhs.rsdt == &rhs.rsdt) && (lhs.idx == rhs.idx);
			}
			friend bool operator!=(const rsdt_t::iterator& lhs, const rsdt_t::iterator& rhs) { return !(lhs == rhs); }

		private:
			const rsdt_t& rsdt;
			usize idx;
			bool is_xsdt;
		};

		union {
			memory::paddr_t xsdt[];
			memory::paddr32_t rsdt[];
		} SDT_ptrs;

		usize ptr_count() {
			auto ptr_table_size = this->length - sizeof(system_description_table_t);
			return ptr_table_size / (this->is_xsdt() ? 8 : 4);
		}

		iterator begin() { return iterator(*this, 0); }
		iterator end() { return iterator(*this, this->ptr_count()); }
		bool is_xsdt() const { return this->has_signature("XSDT"); }
	};

	static_assert(offsetof(rsdt_t, SDT_ptrs.xsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs.rsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs) == 36);

	struct [[gnu::packed]] madt_t : system_description_table_t {};

	template<class VAllocator> struct AcpiData {
		template<class T> using mmap_t = memory::MemoryMap<T, VAllocator>;

		mmap_t<rsdt_t> rsdt;
		std::optional<mmap_t<madt_t>> madt;

		std::vector<Cpu> parse_cpu_info();
	};

	template<class VAllocator>
	AcpiData<VAllocator> parse_acpi_tables(memory::paddr_t rsdt_addr, memory::IPhysicalAllocator& physical_allocator);

	template<class VAllocator>
	AcpiData<VAllocator> parse_acpi_tables(memory::paddr_t rsdt_addr, memory::IPhysicalAllocator& physical_allocator) {
		using enum memory::paging::PageTableFlags;
		constexpr auto acpi_flags = NO_EXECUTE | IMPL_CACHE_WRITETHROUGH | IMPL_CACHE_DISABLE;

		AcpiData<VAllocator> data{
				.rsdt = memory::MemoryMap<rsdt_t, VAllocator>{rsdt_addr,
		                                                      sizeof(system_description_table_t),
		                                                      acpi_flags, physical_allocator,
		                                                      memory::paging::kas}
        };

		LOG(Log::DEBUG, "RSDT has length %llu", data.rsdt->length);
		data.rsdt.resize_to(data.rsdt->length);

		for (auto&& sdt_ptr : *data.rsdt) {
			memory::MemoryMap<system_description_table_t, VAllocator> current{sdt_ptr,
			                                                                  sizeof(system_description_table_t),
			                                                                  acpi_flags,
			                                                                  physical_allocator,
			                                                                  memory::paging::kas};
			current.resize_to(current->length);
			char buf[5] = {0};
			memcpy(buf, current->signature, 4);
			LOG(Log::DEBUG, "Found SDT with signature %s", buf);
			if (current->has_signature("APIC")) { data.madt = static_pointer_cast<madt_t>(std::move(current)); }
		}

		return data;
	}
}   // namespace acpi

#endif   //HUGOS_ACPI_HPP
