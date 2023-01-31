
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_RSDP_HPP
#define HUGOS_RSDP_HPP

#include "multiboot.hpp"

#include <cstddef>

namespace multiboot::tags {
	class [[gnu::packed]] Rsdp : public Tag {
	public:
		char signature[8];
		u8 checksum;
		char oem_id[6];
		u8 revision;
		memory::paddr32_t rsdt_addr_;
		u32 length;
		memory::paddr_t xsdt_addr_;
		u8 extended_checksum;
		std::byte _reserved[3];

		memory::paddr_t rsdt_addr() const {
			if (this->revision == 0) return this->rsdt_addr_;
			else return this->xsdt_addr_;
		}
	};
}   // namespace multiboot::tags

#endif   // HUGOS_RSDP_HPP
