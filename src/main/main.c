#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

#include <termcolor.h>

#include "multiboot.h"
#include "../memory/paging.h"
#include "../memory/frame_bump_alloc.h"
#include "../interrupts/interrupt_handlers.h"
#include <panic.h>

multiboot_data mb;
void rust_test(void*);

void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	if (multiboot_magic == 0x36d76289) {
		kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Mulitboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		kprintf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Mulitboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	multiboot_data_init(&mb, multiboot_addr);
	
	multiboot_tag_framebuffer *fb = (multiboot_tag_framebuffer*)multiboot_data_find_tag(&mb, FRAMEBUFFER);
	multiboot_tag_bootloader *bootloader = (multiboot_tag_bootloader*)multiboot_data_find_tag(&mb, BOOTLOADER_NAME);
	multiboot_tag_cli *cli = (multiboot_tag_cli*)multiboot_data_find_tag(&mb, CLI);
	multiboot_tag_memory_map *mmap = (multiboot_tag_memory_map*)multiboot_data_find_tag(&mb, MEMORY_MAP);
	multiboot_tag_elf_symbols *sections = (multiboot_tag_elf_symbols*)multiboot_data_find_tag(&mb, ELF_SYMBOLS);
	
	if (!mmap) panic("No memory map tag found");

	if (bootloader) kprintf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Booted by %s\n", multiboot_tag_bootloader_get_name(bootloader));

	init_idt();
	kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded IDT\n");


	kprintf("[    ] Initialising memory\n");

	uint64_t kernel_max = 0;
	uint64_t kernel_min = UINT64_MAX;
	for (multiboot_elf_symbols_entry *i = multiboot_tag_elf_symbols_begin(sections); i < multiboot_tag_elf_symbols_end(sections); ++i) {
		if (i->flags != 0) {
			multiboot_elf_symbols_entry_print(i);
			
			if ((i->addr > 0xFFFFFF8000000000 ? i->addr - 0xFFFFFF8000000000 : i->addr) < kernel_min) kernel_min = (i->addr > 0xFFFFFF8000000000 ? i->addr - 0xFFFFFF8000000000 : i->addr);
			if ((i->addr > 0xFFFFFF8000000000 ? i->addr - 0xFFFFFF8000000000 : i->addr) + i->size > kernel_max) kernel_max = (i->addr > 0xFFFFFF8000000000 ? i->addr - 0xFFFFFF8000000000 : i->addr) + i->size;
		}
	}
	kprintf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Kernel executable: %lp -> %lp\n", kernel_min, kernel_max);

	uint64_t available_ram = 0;
	uint64_t total_ram = 0;
	for (multiboot_memory_map_entry *i = multiboot_tag_memory_map_begin(mmap); i < multiboot_tag_memory_map_end(mmap); ++i) {
		if (i->type == AVAILABLE) {
			available_ram += i->length;

			if (i->base_addr + i->length > total_ram) total_ram = i->base_addr + i->length;
		}
	}
	kprintf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Detected %d MiB of available memory (%d MiB total):\n", available_ram / (1024 * 1024), total_ram / (1024 * 1024));

	for (multiboot_memory_map_entry *entry = multiboot_tag_memory_map_begin(mmap); entry < multiboot_tag_memory_map_end(mmap); entry++) {
		kprintf("\t%lp - %lp (%s)\n", entry->base_addr, entry->base_addr + entry->length, entry->type == AVAILABLE ? "AVAILABLE" : "RESERVED");
	}

	frame_bump_alloc_state init_alloc = {
		.vtable = frame_bump_alloc_state_vtable,
		.next_alloc = 0,
		.kernel_start = kernel_min,
		.kernel_end = kernel_max,
		.multiboot_start = (uint64_t)mb.mb_data_start,
		.multiboot_end = (uint64_t)mb.mb_data_end,
		.mem_map = mmap
	};

	uint64_t needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);
	uint64_t needed_frames = IDIV_ROUND_UP(needed_bytes, 0x1000);

	rust_test(&init_alloc);
	while (1);
	

	/*kprintf("[    ] Allocating %u bytes for memory bitmap (%u frames)\n", needed_bytes, needed_frames);
	void *bitmap_frame_starts[needed_frames];
	for (uint64_t i = 0; i < needed_frames; ++i) {
		bitmap_frame_starts[i] = allocator_allocate(&init_alloc.vtable);//frame_bump_alloc_allocate(&init_alloc);
	}

	if (bitmap_frame_starts[needed_frames - 1] > (void*)0x40000000) panic("Could not fit memory bitmap in 1st GiB");

	kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Allocated bitmap frames\n");*/


	while(1);
}
