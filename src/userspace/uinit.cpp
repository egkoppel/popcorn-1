//
// Created by Eliyahu Gluschove-Koppel on 20/09/2022.
//

#include "uinit.hpp"
#include <panic.h>
#include "../threading/threading.hpp"
#include "../initramfs.hpp"
#include "../interrupts/syscall.hpp"
#include "../gdt/tss.hpp"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "dyld.hpp"

#define hugOS_ascii_logo \
" \n\
 _                  ____   _____ \n\
| |                / __ \\ / ____| \n\
| |__  _   _  __ _| |  | | (___ \n\
| '_ \\| | | |/ _` | |  | |\\___ \\ \n\
| | | | |_| | (_| | |__| |____) | \n\
|_| |_|\\__,_|\\__, |\\____/|_____/ \n\
              __/ | \n\
             |___/ \n\
"

Initramfs ramfs;

void dyld_test() {
	syscall(syscall_vectors::print, (uint64_t)"Loading ramfs/loadertest.elf...\n");

	void *loaded_elf;
	size_t loaded_elf_size = ramfs.locate_file("initramfs/loadertest.elf", &loaded_elf);
	Elf64::Elf64File loaded_elf_file((Elf64::Elf64FileHeader *)loaded_elf);
	assert_msg(loaded_elf_size > 0, "Failed to load loadertest.elf");

	auto ret = dyld(loaded_elf_file);

	char buf[64];
	snprintf(buf, 64, "dyld exited with error code %d\n", ret);
	syscall(syscall_vectors::print, (uint64_t)buf);

	while (1);
}

int uinit(Initramfs ramfs_) {
	syscall(syscall_vectors::print, (uint64_t)"\033c");
	syscall(syscall_vectors::print, (uint64_t)hugOS_ascii_logo);
	syscall(syscall_vectors::print, (uint64_t)"Welcome to userspace\n\n");

	ramfs = ramfs_;

	void *ramfs_placeholder_data;
	auto ramfs_placeholder_data_size = ramfs.locate_file("initramfs/.placeholder", &ramfs_placeholder_data);
	assert_msg(ramfs_placeholder_data_size > 0, "Failed to load .placeholder");
	syscall(syscall_vectors::print, (uint64_t)"ramfs .placeholder contents:\n");
	syscall(syscall_vectors::print, (uint64_t)ramfs_placeholder_data);
	syscall(syscall_vectors::print, (uint64_t)"\n");

	syscall(syscall_vectors::serial_write, (uint64_t)"Starting dyld\n");
	syscall(syscall_vectors::print, (uint64_t)"Starting dyld...\n");
	syscall(syscall_vectors::new_proc, (uint64_t)"dyld test", (uint64_t)dyld_test);
	syscall(syscall_vectors::yield);

	while (1);
}

[[noreturn]] int uinit_start(Initramfs ramfs) {
	threads::switch_to_user_mode();
	auto err = uinit(ramfs);
	panic("uinit exited with error code %d\n", err);
}
