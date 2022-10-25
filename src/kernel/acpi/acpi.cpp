/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../memory/kernelspace_map.hpp"
#include "acpi.hpp"

void *RSDT::find_sdt(const char *signature, KernelspaceMapper& page_mapper, Allocator *allocator) {
	auto is_xsdt = strncmp(this->header.signature, "RSDT", 4);

	entry_flags_t flags = {
			.writeable = true,
			.user_accessible = false,
			.write_through = true,
			.cache_disabled = true,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = false,
			.no_execute = true,
	};

	uint64_t entry_count = (this->header.length - sizeof(this->header)) / (is_xsdt ? 8 : 4);
	for (uint64_t i = 0; i < entry_count; i++) {
		uint64_t ptr_phys;
		if (!is_xsdt) {
			ptr_phys = this->SDT_ptrs[i];
		} else {
			ptr_phys = (uint64_t)this->SDT_ptrs[i * 2] | ((uint64_t)this->SDT_ptrs[i * 2 + 1] << 32);
		}

		auto header_addr_virtual = (AcpiSdtHeader *)page_mapper.map_to(ptr_phys, 0x1000, flags, allocator);
		if (!strncmp(header_addr_virtual->signature, signature, 4)) {
			return reinterpret_cast<void *>(header_addr_virtual);
		}
	}
	return nullptr;
}
