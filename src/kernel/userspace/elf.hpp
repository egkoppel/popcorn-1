#ifndef _HUGOS_ELF_H
#define _HUGOS_ELF_H

#include <stdint.h>

namespace Elf64 {
	enum SectionType {
		SHT_NULL = 0,   // Null section
		SHT_PROGBITS = 1,   // Program information
		SHT_SYMTAB = 2,   // Symbol table
		SHT_STRTAB = 3,   // String table
		SHT_RELA = 4,   // Relocation (w/ addend)
		SHT_HASH = 5,   // Symbol hash table
		SHT_DYNAMIC = 6,   // Dynamic linking information
		SHT_NOTE = 7,   // Notes
		SHT_NOBITS = 8,   // Uninitialized space (.bss)
		SHT_REL = 9,   // Relocation (no addend)
		SHT_DYNSYM = 11,  // Dynamic linker symbol table
		SHT_INIT_ARRAY = 14,  // Array of constructors
		SHT_FINI_ARRAY = 15,  // Array of destructors
		SHT_PREINIT_ARRAY = 16,  // Array of pre-constructors
		SHT_GROUP = 17  // Section group
	};

	enum SectionFlags {
		SHF_WRITE = 0x01, // Writable section
		SHF_ALLOC = 0x02, // Exists in memory
		SHF_EXECINSTR = 0x04, // Executable section
	};

	using Elf64_Addr = uint64_t;
	using Elf64_Off = uint64_t;
	using Elf64_Half = uint16_t;
	using Elf64_Word = uint32_t;
	using Elf64_Sword = int32_t;
	using Elf64_Xword = uint64_t;
	using Elf64_Sxword = int64_t;

	class ElfSectionIterator;

	extern "C" struct Elf64SectionHeader {
		Elf64_Word sh_name; /* Section name */
		Elf64_Word sh_type; /* Section type */
		Elf64_Xword sh_flags; /* Section attributes */
		Elf64_Addr sh_addr; /* Virtual address in memory */
		Elf64_Off sh_offset; /* Offset in file */
		Elf64_Xword sh_size; /* Size of section */
		Elf64_Word sh_link; /* Link to other section */
		Elf64_Word sh_info; /* Miscellaneous information */
		Elf64_Xword sh_addralign; /* Address alignment boundary */
		Elf64_Xword sh_entsize; /* Size of entries, if section has table */
	};

	extern "C" struct Elf64FileHeader {
		unsigned char e_ident[16]; /* ELF identification */
		Elf64_Half e_type; /* Object file type */
		Elf64_Half e_machine; /* Machine type */
		Elf64_Word e_version; /* Object file version */
		Elf64_Addr e_entry; /* Entry point address */
		Elf64_Off e_phoff; /* Program header offset */
		Elf64_Off e_shoff; /* Section header offset */
		Elf64_Word e_flags; /* Processor-specific flags */
		Elf64_Half e_ehsize; /* ELF header size */
		Elf64_Half e_phentsize; /* Size of program header entry */
		Elf64_Half e_phnum; /* Number of program header entries */
		Elf64_Half e_shentsize; /* Size of section header entry */
		Elf64_Half e_shnum; /* Number of section header entries */
		Elf64_Half e_shstrndx; /* Section name string table index */

		Elf64SectionHeader *getSectionHeader(int index) {
			return (Elf64SectionHeader *)((uint8_t *)this + e_shoff + index * e_shentsize);
		}

		int verify_header();
	};

	class ElfSectionIterator {
		Elf64FileHeader *header;
		int index;

	public:
		ElfSectionIterator(Elf64FileHeader *header, int index) : header(header), index(index) {}
		Elf64SectionHeader *operator *() {
			return header->getSectionHeader(index);
		}
		ElfSectionIterator& operator ++() {
			index++;
			return *this;
		}
		bool operator !=(const ElfSectionIterator& other) {
			return index != other.index;
		}
	};

	class Elf64File {
	public:
		Elf64FileHeader *header;

	public:
		Elf64File(Elf64FileHeader *header) : header(header) {}

		char *get_str(int index) {
			if (header->e_shstrndx == 0) return nullptr;
			auto str = header->getSectionHeader(header->e_shstrndx);
			return (char *)((uint64_t)header + str->sh_offset + index);
		}

		ElfSectionIterator begin() {
			return ElfSectionIterator(header, 0);
		}

		ElfSectionIterator end() {
			return ElfSectionIterator(header, header->e_shnum);
		}
	};
};

#endif