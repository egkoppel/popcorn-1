
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUGOS_ELF_SECTIONS_HPP
#define HUGOS_ELF_SECTIONS_HPP

#include "multiboot.hpp"

#include <elf/elf64.hpp>
#include <memory/types.hpp>
#include <popcorn_prelude.h>

namespace multiboot::tags {
	class [[gnu::packed]] ElfSections : public Tag {
	public:
		class [[gnu::packed]] Entry {
		private:
			u32 name_index;
			u32 _type;
			u64 _flags;
			memory::paddr_t addr;
			[[maybe_unused]] u64 offset;
			u64 size;
			[[maybe_unused]] u32 link;
			u32 info;
			[[maybe_unused]] u64 align;
			[[maybe_unused]] u64 entry_size;

		public:
			bool operator<=>(const Entry&) const = default;
			bool operator!=(const Entry&) const  = default;
			inline memory::paddr_t start() const { return addr; }
			inline memory::paddr_t end() const { return addr + size; }
			inline Elf64::section_type type() const { return static_cast<Elf64::section_type>(this->_type); }
			inline u64 flags() const { return this->_flags; }
			char *name(ElfSections& sections) {
				return static_cast<char *>(sections.find_strtab()->start().virtualise() + this->name_index);
			}

			friend FILE *operator<<(FILE *f, const multiboot::tags::ElfSections::Entry& entry) {
				fprintf(f,
				        "ElfSection { PhysAddr=%lp, VirtAddr=%lp, Size=%llu, Flags=0x%llx }\n",
				        (entry.addr.address > 0xFFFFFFFF80000000 ? entry.addr - 0xFFFFFFFF80000000 : entry.addr),
				        entry.addr,
				        entry.size,
				        entry.flags());
				return f;
			}

			friend char *operator<<(char *c, const multiboot::tags::ElfSections::Entry& entry) {
				sprintf(c,
				        "ElfSection { PhysAddr=%lp, VirtAddr=%lp, Size=%llu, Flags=0x%llx }\n",
				        (entry.addr.address > 0xFFFFFFFF80000000 ? entry.addr - 0xFFFFFFFF80000000 : entry.addr),
				        entry.addr,
				        entry.size,
				        entry.flags());
				return c;
			}
		};

	private:
		u32 entry_count;
		u32 entry_size;
		u32 string_table;
		Entry sections[];

	public:
		ElfSections(const ElfSections& other) = delete;

		inline Entry *begin() { return &this->sections[0]; }
		inline Entry *end() { return &this->sections[this->entry_count]; }
		Entry *find_strtab() {
			u32 i = 0;
			for (auto& section : *this) {
				if (i == this->string_table) return &section;
				i++;
			}
			return nullptr;
		}
	};
}   // namespace multiboot::tags

u64 operator+(Elf64::section_flags lhs) {
	return static_cast<uint64_t>(lhs);
}


#endif   // HUGOS_ELF_SECTIONS_HPP
