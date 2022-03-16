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

allocator_vtable *global_frame_allocator = NULL;

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
		if ((i->type != SHT_NULL) && (i->flags & SHF_ALLOC) != 0) {
			multiboot_elf_symbols_entry_print(i);
			
			if (i->addr - 0xFFFFFF8000000000 < kernel_min) kernel_min = i->addr - 0xFFFFFF8000000000;
			if (i->addr - 0xFFFFFF8000000000 + i->size > kernel_max) kernel_max = i->addr - 0xFFFFFF8000000000 + i->size;
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
	global_frame_allocator = &init_alloc.vtable;

	uint64_t needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);
	uint64_t needed_frames = IDIV_ROUND_UP(needed_bytes, 0x1000);

	uint64_t new_p4_table = create_p4_table(&init_alloc.vtable);

	uint64_t multiboot_virt_addr, memory_bitmap;
	{
		mapper_ctx_t ctx = mapper_ctx_begin(new_p4_table, global_frame_allocator);

		// Map the kernel with section flags
		for (multiboot_elf_symbols_entry *i = multiboot_tag_elf_symbols_begin(sections); i < multiboot_tag_elf_symbols_end(sections); ++i) {
			if ((i->type != SHT_NULL) && (i->flags & SHF_ALLOC) != 0) {
				for (uint64_t phys_addr = i->addr - 0xFFFFFF8000000000; phys_addr < ALIGN_UP(i->addr - 0xFFFFFF8000000000 + i->size, 0x1000); phys_addr+=0x1000) {
					map_page_to(phys_addr + 0xFFFFFF8000000000, phys_addr, global_frame_allocator);

					entry_flags_t flags = {
						.accessed = 0,
						.cache_disabled = 0,
						.dirty = 0,
						.global = 1,
						.huge = 0,
						.no_execute = !(i->flags & SHF_EXECINSTR),
						.user_accessible = 0,
						.write_through = 0,
						.writeable = i->flags & SHF_WRITE
					};

					set_entry_flags_for_address(phys_addr + 0xFFFFFF8000000000, flags);
				}
			}
		}

		// Map the framebuffer
		uint64_t virt_addr = 0xFFFFFF8040000000;
		for (uint64_t phys_addr = fb->addr; phys_addr < ALIGN_UP(fb->addr + fb->height*fb->pitch, 0x1000); phys_addr+=0x1000, virt_addr+=0x1000) {
			map_page_to(virt_addr, phys_addr, global_frame_allocator);

			entry_flags_t flags = {
				.accessed = 0,
				.cache_disabled = 0,
				.dirty = 0,
				.global = 1,
				.huge = 0,
				.no_execute = 1,
				.user_accessible = 0,
				.write_through = 0,
				.writeable = 1
			};

			set_entry_flags_for_address(virt_addr, flags);
		}

		// Map multiboot
		uint64_t multiboot_phys_addr = ALIGN_DOWN(multiboot_addr, 0x1000);
		multiboot_virt_addr = ALIGN_UP(0xFFFFFF8040000000 + fb->height*fb->pitch, 0x1000); 
		uint64_t multiboot_virt_end = ALIGN_UP(multiboot_virt_addr + (uint64_t)mb.mb_data_end - (uint64_t)mb.mb_data_start, 0x1000);
		for (; multiboot_virt_addr < multiboot_virt_end; multiboot_virt_addr+=0x1000, multiboot_phys_addr+=0x1000) {
			map_page_to(multiboot_virt_addr, multiboot_phys_addr, global_frame_allocator);

			entry_flags_t flags = {
				.accessed = 0,
				.cache_disabled = 0,
				.dirty = 0,
				.global = 0,
				.huge = 0,
				.no_execute = 1,
				.user_accessible = 0,
				.write_through = 0,
				.writeable = 0
			};

			set_entry_flags_for_address(multiboot_virt_addr, flags);
		}
		multiboot_virt_addr = ALIGN_UP(0xFFFFFF8040000000 + fb->height*fb->pitch, 0x1000) + (multiboot_addr - ALIGN_DOWN(multiboot_addr, 0x1000));

		// Map bitmap
		memory_bitmap = multiboot_virt_end;
		kprintf("[    ] Allocating %u bytes for memory bitmap (%u frames) at %p\n", needed_bytes, needed_frames, memory_bitmap);
		for (uint64_t i = 0; i < needed_frames; ++i) {
			map_page(memory_bitmap + i*0x1000, global_frame_allocator);
		}
		kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Allocated memory bitmap (ends at %p)\n", memory_bitmap + needed_frames*0x1000);
		
		mapper_ctx_end(ctx);
	}

	uint64_t old_p4_table_page;
	__asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_page));
	old_p4_table_page += 0xFFFFFF8000000000;
	__asm__ volatile("mov %0, %%cr3" : : "r"(new_p4_table));

	kfprintf(stdserial, "Creating stack guard page at %lp\n", old_p4_table_page);
	unmap_page_no_free(old_p4_table_page);

	kfprintf(stdout, "[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Reloaded page tables\n");

	while(1);

	/*kprintf("[    ] Allocating %u bytes for memory bitmap (%u frames)\n", needed_bytes, needed_frames);
	void *bitmap_frame_starts[needed_frames];
	for (uint64_t i = 0; i < needed_frames; ++i) {
		bitmap_frame_starts[i] = allocator_allocate(&init_alloc.vtable);//frame_bump_alloc_allocate(&init_alloc);
	}

	if (bitmap_frame_starts[needed_frames - 1] > (void*)0x40000000) panic("Could not fit memory bitmap in 1st GiB");

	kprintf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Allocated bitmap frames\n");*/
}
