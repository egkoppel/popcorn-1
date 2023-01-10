/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_ACPI_HPP
#define HUGOS_ACPI_HPP

#include "sdts/apic.hpp"
#include "sdts/rsdt.hpp"
#include "sdts/sdt.hpp"

#include <cstdint>
#include <cstring>
#include <map>
#include <memory/memory_map.hpp>
#include <optional>
#include <smp/cpu.hpp>
#include <string>
#include <tuple>
#include <vector>

namespace acpi {
	template<class VAllocator> struct AcpiData {
		template<class T> using mmap_t = memory::MemoryMap<T, VAllocator>;

		mmap_t<rsdt_t> rsdt;
		std::optional<mmap_t<madt_t>> madt;

		std::tuple<std::vector<Cpu>> parse_cpu_info(memory::IPhysicalAllocator& allocator);
	};

	template<class VAllocator>
	AcpiData<VAllocator> parse_acpi_tables(memory::paddr_t rsdt_addr, memory::IPhysicalAllocator& physical_allocator);
}   // namespace acpi

#include "acpi.ipp"

#endif   // HUGOS_ACPI_HPP
