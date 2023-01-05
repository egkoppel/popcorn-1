/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_ACPI_ACPI_IPP
#define POPCORN_KERNEL_SRC_ACPI_ACPI_IPP

#include "sdts/apic.hpp"

namespace acpi {
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

			char buf[5] = {0};
			memcpy(buf, current->signature, 4);
			LOG(Log::INFO, "Found SDT with signature %s of length %x", buf, current->length);

			if (current->has_signature("APIC")) {
				current.resize_to(current->length);
				data.madt = static_pointer_cast<madt_t>(std::move(current));
			}
		}

		return data;
	}

	template<class VAllocator>
	std::tuple<std::vector<Cpu>> AcpiData<VAllocator>::parse_cpu_info(memory::IPhysicalAllocator& allocator) {
		using enum memory::paging::PageTableFlags;
		constexpr auto lapic_flags = WRITEABLE | NO_EXECUTE | GLOBAL | IMPL_CACHE_WRITETHROUGH | IMPL_CACHE_DISABLE;
		auto lapic_addr            = this->madt.value()->lapic_addr();
		std::vector<Cpu> cpus;

		for (auto&& entry_raw : *this->madt.value()) {
			switch (entry_raw.type) {
				using enum madt::entry_type;
				case CPU_LAPIC: {
					auto entry = static_cast<madt::lapic_entry_t&>(entry_raw);
					LOG(Log::INFO, "Found CPU with APIC ID: %u\nflags: %02b", entry.apic_id, entry.flags);
					if ((entry.flags & 1) || (entry.flags & 2)) { cpus.push_back({entry.apic_id}); }
					break;
				}
				case IO_APIC: {
					auto entry = static_cast<madt::ioapic_entry_t&>(entry_raw);
					break;
				}
				case LAPIC_ADDR: {
					auto entry = static_cast<madt::lapic_addr_entry_t&>(entry_raw);
					lapic_addr = entry.addr;
					break;
				}
				default: break;
			}
		}
		LOG(Log::INFO, "LAPIC is at %lp", lapic_addr);
		Cpu::lapic = memory::MemoryMap<volatile lapic>{lapic_addr,
		                                               sizeof(volatile lapic),
		                                               lapic_flags,
		                                               allocator,
		                                               memory::paging::kas};
		return std::make_tuple(std::move(cpus));
	}
}   // namespace acpi

#endif   // POPCORN_KERNEL_SRC_ACPI_ACPI_IPP