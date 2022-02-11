#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

#include <termcolor.h>

#include "multiboot.hpp"

extern "C" int init_serial();

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	if (multiboot_magic == 0x36d76289) {
		kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Mulitboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		kprintf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Mulitboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	init_serial();

	MultibootData mb(multiboot_addr);
	//mb.print_tags();
	multiboot::tag_framebuffer* fb = mb.find_tag(multiboot::tag_types::FRAMEBUFFER).framebuffer;
	multiboot::tag_bootloader* bootloader = mb.find_tag(multiboot::tag_types::BOOTLOADER_NAME).bootloader;
	multiboot::tag_cli* cli = mb.find_tag(multiboot::tag_types::CLI).cli;
	multiboot::tag_memory_map* mmap = mb.find_tag(multiboot::tag_types::MEMORY_MAP).memory_map;
	if (!fb) kprintf("No fb tag found");
	else {
		kprintf("Framebuffer: %ux%u\n", fb->width, fb->height);
		kprintf("\tAddress: 0x%llx\n", fb->addr);
		kprintf("\tPitch: %u\n", fb->pitch);
		kprintf("\tbpp: %u\n", fb->bpp);
	}

	if (!bootloader) kprintf("No bootloader tag found");
	else {
		kprintf("Bootloader name: ");
		kprintf(bootloader->get_name());
		kprintf("\n");
	}

	if (!cli) kprintf("No cli tag found");
	else {
		kprintf("CLI args: ");
		kprintf(cli->get_str());
		kprintf("\n");
	}

	if (!mmap) kprintf("No mmap tag found");
	else {
		kprintf("Memory map:\n");
		for (auto &entry : *mmap) {
			kprintf("\t%p -> %p (%s)\n", entry.base_addr, entry.base_addr + entry.length, entry.type == multiboot::memory_map_entry::AVAILABLE ? "AVAILABLE" : "RESERVED");
		}
	}

	kfprintf(stdserial, "Hello world!\n");
	kfprintf(stdserial, "Framebuffer:\n");
	kfprintf(stdserial, "\tAddress: 0x%llx\n", fb->addr);
	kfprintf(stdserial, "\tPitch: %u\n", fb->pitch);
	kfprintf(stdserial, "\tbpp: %u\n", fb->bpp);
	
	while(1);
}
