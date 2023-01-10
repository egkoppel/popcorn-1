
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_ACPI_SDT_HPP
#define POPCORN_KERNEL_SRC_ACPI_SDT_HPP

#include <cstring>
#include <popcorn_prelude.h>

namespace acpi {
	struct [[gnu::packed]] system_description_table_t {
		char signature[4];
		u32 length;
		u8 revision;
		u8 checksum;
		char oem_id[6];
		char oem_table_id[8];
		u32 oem_revision;
		u32 creator_id;
		u32 creator_revision;

		bool has_signature(const char *s) const noexcept { return !std::strncmp(s, this->signature, 4); }
	};
}   // namespace acpi

#endif   //POPCORN_KERNEL_SRC_ACPI_SDT_HPP
