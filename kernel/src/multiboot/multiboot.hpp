/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_MULTIBOOT_HPP
#define HUGOS_MULTIBOOT_HPP

#include "memory.h"
#include "memory/types.hpp"
#include "utils.h"

#include <optional>
#include <stdint.h>
#include <stdlib.h>

namespace multiboot {
	enum class TagType : uint32_t {
		CLI             = 1,
		BOOTLOADER_NAME = 2,
		BOOT_MODULE     = 3,
		MEMORY_MAP      = 6,
		FRAMEBUFFER     = 8,
		ELF_SECTIONS    = 9,
		RSDT_V1         = 14,
		RSDT_V2         = 15
	};

	class [[gnu::packed]] InfoHeader {
	protected:
		uint32_t size_;
		uint32_t reserved;
	};

	class [[gnu::packed]] Tag {
	protected:
		TagType type;
		uint32_t size;

	public:
		inline TagType get_type() const { return this->type; }
		inline uint32_t get_size() const { return this->size; }
	};

	class Data : private InfoHeader {
	private:
		memory::vaddr_t tags_begin() const { return this->begin() + sizeof(*this); }

	public:
		Data(const Data&) = delete;

		template<class T> std::optional<T *> find_tag(TagType type) const {
			auto current_tag = static_cast<Tag *>(this->tags_begin());
			while (current_tag < static_cast<Tag *>(this->end())) {
				if (current_tag->get_type() == type) { return {reinterpret_cast<T *>(current_tag)}; }
				current_tag = ADD_BYTES(current_tag, current_tag->get_size());
				current_tag = ALIGN_UP(current_tag, 8);
			}
			return std::nullopt;
		}

		memory::vaddr_t begin() const { return {.address = reinterpret_cast<usize>(this)}; }
		memory::vaddr_t end() const { return this->begin() + this->size_; }
		size_t size() { return this->size_; }
	};
}   // namespace multiboot

#endif   //HUGOS_MULTIBOOT_HPP
