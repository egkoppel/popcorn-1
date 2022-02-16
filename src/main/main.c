#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

#include <termcolor.h>

#include "multiboot.h"
#include "../memory/paging.h"
#include "../interrupts/interrupt_handlers.h"

void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	if (multiboot_magic == 0x36d76289) {
		kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Mulitboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		kprintf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Mulitboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	multiboot_data mb;
	multiboot_data_init(&mb, multiboot_addr);
	
	multiboot_tag_framebuffer *fb = (multiboot_tag_framebuffer*)multiboot_data_find_tag(&mb, FRAMEBUFFER);
	multiboot_tag_bootloader *bootloader = (multiboot_tag_bootloader*)multiboot_data_find_tag(&mb, BOOTLOADER_NAME);
	multiboot_tag_cli *cli = (multiboot_tag_cli*)multiboot_data_find_tag(&mb, CLI);
	multiboot_tag_memory_map *mmap = (multiboot_tag_memory_map*)multiboot_data_find_tag(&mb, MEMORY_MAP);
	
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
		kprintf(multiboot_tag_bootloader_get_name(bootloader));
		kprintf("\n");
	}

	if (!cli) kprintf("No cli tag found");
	else {
		kprintf("CLI args: ");
		kprintf("%c\n", cli->str);
		kprintf(multiboot_tag_cli_get_str(cli));
		kprintf("\n");
	}

	if (!mmap) kprintf("No mmap tag found");
	else {
		kprintf("Memory map:\n");
		for (multiboot_memory_map_entry *entry = multiboot_tag_memory_map_begin(mmap);
				entry < multiboot_tag_memory_map_end(mmap);
				entry++) {
			kprintf("\t%p -> %p (%s)\n", entry->base_addr, entry->base_addr + entry->length, entry->type == 1 ? "AVAILABLE" : "RESERVED");
		}
	}

	kfprintf(stdserial, "Hello world!\n");
	kfprintf(stdserial, "Framebuffer:\n");
	kfprintf(stdserial, "\tAddress: 0x%llx\n", fb->addr);
	kfprintf(stdserial, "\tPitch: %u\n", fb->pitch);
	kfprintf(stdserial, "\tbpp: %u\n", fb->bpp);
	kfprintf(stdserial, "\tRed mask: 0x%x\n", fb->color_info.rgb.red_mask_size);
	kfprintf(stdserial, "\tRed pos: 0x%x\n", fb->color_info.rgb.red_pos);
	kfprintf(stdserial, "\tGreen mask: 0x%x\n", fb->color_info.rgb.green_mask_size);
	kfprintf(stdserial, "\tGreen pos: 0x%x\n", fb->color_info.rgb.green_pos);
	kfprintf(stdserial, "\tBlue mask: 0x%x\n", fb->color_info.rgb.blue_mask_size);
	kfprintf(stdserial, "\tBlue pos: 0x%x\n", fb->color_info.rgb.blue_pos);


	multiboot_data_print_tags(&mb);

	kprintf("%s\n", cli);

	init_idt();

	while(1);
}
