
/*
 * Copyright (c) 2023 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POPCORN_KERNEL_SRC_ELF_ELF64_HPP
#define POPCORN_KERNEL_SRC_ELF_ELF64_HPP

#include <popcorn_prelude.h>

namespace Elf64 {
	enum class section_type : u32 {
		SHT_NULL,
		SHT_PROGBITS,
		SHT_SYMTAB,
		SHT_STRTAB,
		SHT_RELA,
		SHT_HASH,
		SHT_DYNAMIC,
		SHT_NOTE,
		SHT_NOBITS,
		SHT_REL,
		SHT_SHLIB,
		SHT_DYNSYM,
		SHT_INIT_ARRAY,
		SHT_FINI_ARRAY,
		SHT_PREINIT_ARRAY,
		SHT_GROUP,
		SHT_SYMTAB_SHNDX,
		SHT_NUM
	};

	enum class section_flags : u64 {
		SHF_WRITE            = 0x1,
		SHF_ALLOC            = 0x2,
		SHF_EXECINSTR        = 0x4,
		SHF_MERGE            = 0x10,
		SHF_STRINGS          = 0x20,
		SHF_INFO_LINK        = 0x40,
		SHF_LINK_ORDER       = 0x80,
		SHF_OS_NONCONFORMING = 0x100,
		SHF_GROUP            = 0x200,
		SHF_TLS              = 0x400,
		SHF_MASKOS           = 0x0ff00000,
		SHF_MASKPROC         = 0xf0000000
	};

	constexpr u64 operator+(Elf64::section_flags lhs) {
        	return static_cast<uint64_t>(lhs);
	}

	enum class segment_type : u32 {
		PT_NULL    = 0,
		PT_LOAD    = 1,
		PT_DYNAMIC = 2,
		PT_INTERP  = 3,
		PT_NOTE    = 4,
		PT_SHLIB   = 5,
		PT_PHDR    = 6,
		PT_LOOS    = 0x6000'0000,
		PT_HIOS    = 0x6fff'ffff,
		PT_LOPROC  = 0x7000'0000,
		PT_HIPROC  = 0x7fff'ffff,
	};

	enum class segment_flags : u32 { PF_X = 1 << 0, PF_W = 1 << 1, PF_R = 1 << 2 };
}   // namespace Elf64

#endif   // POPCORN_KERNEL_SRC_ELF_ELF64_HPP
