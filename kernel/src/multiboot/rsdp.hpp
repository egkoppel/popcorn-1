
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
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

namespace multiboot::tags {
	class [[gnu::packed]] Rsdp : public Tag {
	public:
		char signature[8];
		uint8_t checksum;
		char oem_id[6];
		uint8_t revision;
		uint32_t rsdt_addr;
		uint32_t length;
		uint64_t xsdt_addr;
		uint8_t extended_checksum;
		uint8_t _reserved[3];
	};
}

#endif //HUGOS_RSDP_HPP
