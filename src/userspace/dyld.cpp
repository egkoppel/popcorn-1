//
// Created by Eliyahu Gluschove-Koppel on 21/09/2022.
//

#include "dyld.hpp"
#include "../interrupts/syscall.hpp"
#include <string.h>
#include <utils.h>

int dyld(Elf64::Elf64File file) {
	if (file.header->verify_header()) return -1;

	for (auto section : file) {
		char *section_name = file.get_str(section->sh_name);
		if (section->sh_flags & Elf64::SHF_ALLOC) {
			uint64_t section_page_start = ALIGN_DOWN(section->sh_addr, 0x1000);
			uint64_t section_page_size = section->sh_size;

			uint64_t flags = (mmap_prot::PROT_READ) |
					         (mmap_prot::PROT_WRITE) |
							 (mmap_prot::PROT_EXEC * (section->sh_flags & Elf64::SHF_EXECINSTR));
			auto actual_start = syscall(syscall_vectors::mmap_anon, section_page_start, section_page_size | flags);
			if (actual_start != section_page_start) return -2;

			memcpy(reinterpret_cast<void*>(section->sh_addr), (void*)((uint64_t)file.header + section->sh_offset), section->sh_size);
		}
	}

	auto _start = reinterpret_cast<int(*)(void)>(file.header->e_entry);
	return _start();
}