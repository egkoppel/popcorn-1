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
	std::vector<Cpu> parse_cpu_info();

	/*private:
		const RootSystemDescriptionTable& rsdt;
		mutable std::map<std::string, SystemDescriptionTable&> table_address_cache;
		bool is_xsdt;

	public:
		explicit RootSystemDescriptionTableReader(const RootSystemDescriptionTable& rsdt) noexcept : rsdt(rsdt) {
			this->is_xsdt = strncmp(this->rsdt.signature, "RSDT", 4);
		}

		template<class SDT>
			requires(std::is_base_of_v<SystemDescriptionTable, SDT> && requires(SDT tab) {
				tab.signature;
			})
		std::optional<SystemDescriptionTableReader<SDT>> find_sdt(memory::IVirtualAllocator& page_allocator,
		                                                 memory::IPhysicalAllocator& frame_allocator) const {

		}
	};*/
}   // namespace acpi

#endif   //HUGOS_ACPI_HPP
