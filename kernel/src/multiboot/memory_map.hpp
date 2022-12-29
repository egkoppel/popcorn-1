
/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MEMORY_MAP_HPP
#define HUGOS_MEMORY_MAP_HPP

#include "multiboot.hpp"

#include <memory/types.hpp>

namespace multiboot::tags {
	class [[gnu::packed]] MemoryMap : public Tag {
	public:
		enum class Type : uint32_t { RESERVED = 0, AVAILABLE = 1, ACPI = 3, HIBERNATION_SAVE = 4, DEFECTIVE = 5 };

		class [[gnu::packed]] Entry {
		private:
			memory::paddr_t base_addr;
			uint64_t length;
			Type type;
			uint32_t reserved;

		public:
			bool operator==(const Entry& rhs) const { return this->base_addr == rhs.base_addr; };
			bool operator!=(const Entry& rhs) const = default;
			inline memory::paddr_t get_start_address() const { return this->base_addr; }
			inline memory::paddr_t get_end_address() const { return this->base_addr + this->length; }
			inline Type get_type() const { return this->type; }
			inline uint64_t get_size() const { return this->length; }
		};

	private:
		uint32_t entry_size;
		uint32_t entry_version;
		Entry first_entry;

	public:
		MemoryMap(const MemoryMap&) = delete;

		inline Entry *begin() { return &this->first_entry; }
		inline Entry *end() { return reinterpret_cast<Entry *>(ADD_BYTES(this, this->size)); }
	};
}   // namespace multiboot::tags

#endif   //HUGOS_MEMORY_MAP_HPP
