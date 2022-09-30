/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUG_ACPI_HPP
#define HUG_ACPI_HPP

#include <stdint.h>
#include <string.h>
#include "../memory/kernelspace_map.hpp"

struct AcpiSdtHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

extern "C" struct RSDT {
	AcpiSdtHeader header;
	uint32_t SDT_ptrs[];
	void *find_sdt(const char *signature, KernelspaceMapper& page_mapper, allocator_vtable *allocator);
} __attribute__((packed));

#endif //HUG_ACPI_HPP
