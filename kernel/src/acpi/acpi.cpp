/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "acpi.hpp"

#include "../memory/memory_map.hpp"

#include <optional>
#include <smp/cpu.hpp>

using namespace memory;

/*std::optional<SystemDescriptionTable *>
RootSystemDescriptionTableReader::find_sdt(const char *signature,
                                           memory::IVirtualAllocator& page_allocator,
                                           memory::IPhysicalAllocator& frame_allocator) const {
	if (auto ptr = this->table_address_cache.find(signature); ptr != this->table_address_cache.end()) {
		return {&ptr->second};
	}

	using enum memory::paging::PageTableFlags;
	auto flags = IMPL_CACHE_DISABLE | IMPL_CACHE_WRITETHROUGH | NO_EXECUTE;

	uint64_t entry_count = (this->rsdt.length - sizeof(SystemDescriptionTable)) / (is_xsdt ? 8 : 4);
	for (uint64_t i = 0; i < entry_count; i++) {
		memory::paddr_t ptr_phys{};

		if (is_xsdt) {
			ptr_phys = this->rsdt.SDT_ptrs.xsdt[i];
		} else {
			ptr_phys = this->rsdt.SDT_ptrs.rsdt[i];
		}

		memory::MemoryMap header{ptr_phys, 0x1000, flags, frame_allocator};


		if (header_addr_virtual.has_signature(signature)) {
			this->table_address_cache.insert({signature, header_addr_virtual});
			return {&header_addr_virtual};
		}
	}
	return std::nullopt;
}*/

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

		bool has_signature(const char *s) noexcept { return !strncmp(s, this->signature, 4); }
	};

	struct [[gnu::packed]] rsdt_t : system_description_table_t {
		class iterator {
		public:
			paddr_t operator*();
		};

		union {
			paddr_t xsdt[];
			paddr32_t rsdt[];
		} SDT_ptrs;

		iterator begin();
		iterator end();
	};

	static_assert(offsetof(rsdt_t, SDT_ptrs.xsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs.rsdt) == 36);
	static_assert(offsetof(rsdt_t, SDT_ptrs) == 36);

	struct [[gnu::packed]] madt_t : system_description_table_t {};

	template<class VAllocator> struct AcpiData {
		template<class T> using mmap_t = MemoryMap<T, VAllocator>;

		std::optional<mmap_t<rsdt_t>> rsdt;
		std::optional<mmap_t<madt_t>> madt;
	};

	namespace {
		using enum memory::paging::PageTableFlags;
		constexpr auto acpi_flags = NO_EXECUTE | IMPL_CACHE_WRITETHROUGH | IMPL_CACHE_DISABLE;
	}   // namespace

	paddr_t find_sdt(const char *signature);

	template<class VAllocator>
	AcpiData<VAllocator> parse_acpi_tables(paddr_t rsdt_addr, IPhysicalAllocator& physical_allocator) {
		AcpiData<VAllocator> data{};
		data.rsdt = MemoryMap{rsdt_addr, sizeof(system_description_table_t), acpi_flags, physical_allocator};
		data.rsdt->expand(data.rsdt->length);

		for (auto&& sdt_ptr : *data.rsdt) {
			/* TODO: Should this using some kind of "null" physical allocator since in theory it's all MMIO and therefore not covered by any memory allocators */
			MemoryMap<system_description_table_t, VAllocator> current{sdt_ptr,
			                                                          constants::frame_size,
			                                                          acpi_flags,
			                                                          physical_allocator};

			if (current->has_signature("APIC")) {
				data.madt = std::move(static_cast<MemoryMap<madt_t, VAllocator>>(current));
			}
		}
	}

	std::vector<Cpu> parse_cpu_info() { return {}; }
}   // namespace acpi
