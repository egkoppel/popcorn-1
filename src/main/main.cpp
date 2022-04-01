#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <termcolor.h>

#include "multiboot.hpp"
#include "../memory/memory.h"
#include "../memory/paging.h"
#include "../memory/stack.hpp"
#include "../memory/frame_bump_alloc.hpp"
#include "../memory/frame_main_alloc.hpp"
#include "../interrupts/interrupt_handlers.hpp"
#include "../gdt/gdt.hpp"
#include "../gdt/tss.hpp"
#include "../initramfs.hpp"
#include "../threading/threading.hpp"

#include <panic.h>

extern "C" allocator_vtable *global_frame_allocator = nullptr;

extern "C" void switch_to_user_mode(void);

void stackoveflow();
void stackoveflow() {
	stackoveflow();
	__asm__ volatile("nop");
}

void test_task(uint64_t a, uint64_t b, uint64_t c) {
	printf("Test task!\na: %llu, b: %llu\n", a, b);
	threads::scheduler.lock().schedule();
	printf("Back in test task\n");
	while(1);
}

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	if (multiboot_magic == 0x36d76289) {
		printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Mulitboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		printf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Mulitboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	multiboot::Data mb(multiboot_addr);
	
	multiboot::framebuffer_tag *fb = mb.find_tag<multiboot::framebuffer_tag>(multiboot::tag_type::FRAMEBUFFER);
	multiboot::bootloader_tag *bootloader = mb.find_tag<multiboot::bootloader_tag>(multiboot::tag_type::BOOTLOADER_NAME);
	multiboot::cli_tag *cli = mb.find_tag<multiboot::cli_tag>(multiboot::tag_type::CLI);
	multiboot::memory_map_tag *mmap = mb.find_tag<multiboot::memory_map_tag>(multiboot::tag_type::MEMORY_MAP);
	multiboot::elf_sections_tag *sections = mb.find_tag<multiboot::elf_sections_tag>(multiboot::tag_type::ELF_SECTIONS);
	multiboot::boot_module_tag *boot_module = mb.find_tag<multiboot::boot_module_tag>(multiboot::tag_type::BOOT_MODULE);
	
	if (!mmap) panic("No memory map tag found");

	if (bootloader) printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Booted by %s\n", bootloader->get_name());

	if (!boot_module) panic("No initramfs found");
	if (strcmp(boot_module->get_name(), "initramfs") != 0) panic("No initramfs found");

	init_idt();
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded IDT\n");
	
	gdt::GDT global_descriptor_table = gdt::GDT();
	global_descriptor_table.add_entry(gdt::entry::new_code_segment(0)); // Kernel code
	global_descriptor_table.add_entry(gdt::entry::new_data_segment(0)); // Kernel data
	global_descriptor_table.add_entry(gdt::entry()); // [Unused] - compatibility mode user code
	global_descriptor_table.add_entry(gdt::entry::new_data_segment(3)); // User data
	global_descriptor_table.add_entry(gdt::entry::new_code_segment(3)); // User code
	global_descriptor_table.load();
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded GDT\n");

	// Load STAR and LSTAR registers with syscall handler
	__asm__ volatile("mov $0xC0000081, %%ecx; mov $0, %%eax; mov $0x001B0008, %%edx; wrmsr;" : : : "%eax", "%edx", "%ecx"); // STAR
	uint32_t handler_low = (uint64_t)syscall_long_mode_handler;
	uint32_t handler_high = (uint64_t)syscall_long_mode_handler >> 32;
	__asm__ volatile("mov $0xC0000082, %%ecx; wrmsr;" : : "d"(handler_high), "a"(handler_low) : "%ecx"); // LSTAR

	printf("[    ] Initialising memory\n");

	uint64_t kernel_max = 0;
	uint64_t kernel_min = UINT64_MAX;
	for (multiboot::elf_sections_entry i : *sections) {
		if ((i.type != SHT_NULL) && (i.flags & SHF_ALLOC) != 0) {
			i.print();
			
			if (i.addr - 0xFFFFFF8000000000 < kernel_min) kernel_min = i.addr - 0xFFFFFF8000000000;
			if (i.addr - 0xFFFFFF8000000000 + i.size > kernel_max) kernel_max = i.addr - 0xFFFFFF8000000000 + i.size;
		}
	}
	printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Kernel executable: %lp -> %lp\n", kernel_min, kernel_max);

	uint64_t available_ram = 0;
	uint64_t total_ram = 0;
	for (multiboot::memory_map_entry i : *mmap) {
		if (i.type == multiboot::memory_type::AVAILABLE) {
			available_ram += i.length;

			if (i.base_addr + i.length > total_ram) total_ram = i.base_addr + i.length;
		}
	}
	printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Detected %d MiB of available memory (%d MiB total):\n", available_ram / (1024 * 1024), total_ram / (1024 * 1024));

	for (multiboot::memory_map_entry entry : *mmap) {
		printf("\t%lp - %lp (%s)\n", entry.base_addr, entry.base_addr + entry.length, entry.type == multiboot::memory_type::AVAILABLE ? "AVAILABLE" : "RESERVED");
	}

	frame_bump_alloc_state init_alloc = {
		.vtable = frame_bump_alloc_state_vtable,
		.next_alloc = 0,
		.kernel_start = kernel_min,
		.kernel_end = kernel_max,
		.multiboot_start = reinterpret_cast<uint64_t>(mb.mb_data_start),
		.multiboot_end = reinterpret_cast<uint64_t>(mb.mb_data_end),
		.mem_map = mmap
	};
	global_frame_allocator = &init_alloc.vtable;

	uint64_t needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);
	uint64_t needed_frames = IDIV_ROUND_UP(needed_bytes, 0x1000);

	uint64_t new_p4_table = create_p4_table(&init_alloc.vtable);

	uint64_t multiboot_virt_addr, memory_bitmap, initramfs_virt_addr;
	{
		mapper_ctx_t ctx = mapper_ctx_begin(new_p4_table, global_frame_allocator);

		// Map the kernel with section flags
		for (multiboot::elf_sections_entry i : *sections) {
			if ((i.type != SHT_NULL) && (i.flags & SHF_ALLOC) != 0) {
				for (uint64_t phys_addr = i.addr - 0xFFFFFF8000000000; phys_addr < ALIGN_UP(i.addr - 0xFFFFFF8000000000 + i.size, 0x1000); phys_addr+=0x1000) {
					entry_flags_t flags = {
						.writeable = static_cast<bool>(i.flags & SHF_WRITE),
						.user_accessible = 1,
						.write_through = 0,
						.cache_disabled = 0,
						.accessed = 0,
						.dirty = 0,
						.huge = 0,
						.global = 1,
						.no_execute = !(i.flags & SHF_EXECINSTR)
					};

					map_page_to(phys_addr + 0xFFFFFF8000000000, phys_addr, flags, global_frame_allocator);
				}
			}
		}

		// Map the framebuffer
		uint64_t virt_addr = 0xFFFFFF8040000000;
		for (uint64_t phys_addr = fb->addr; phys_addr < ALIGN_UP(fb->addr + fb->height*fb->pitch, 0x1000); phys_addr+=0x1000, virt_addr+=0x1000) {
			entry_flags_t flags = {
				.writeable = 1,
				.user_accessible = 1,
				.write_through = 0,
				.cache_disabled = 0,
				.accessed = 0,
				.dirty = 0,
				.huge = 0,
				.global = 1,
				.no_execute = 1
			};

			map_page_to(virt_addr, phys_addr, flags, global_frame_allocator);
		}

		// Map multiboot
		uint64_t multiboot_phys_addr = ALIGN_DOWN(multiboot_addr, 0x1000);
		multiboot_virt_addr = ALIGN_UP(0xFFFFFF8040000000 + fb->height*fb->pitch, 0x1000); 
		uint64_t multiboot_virt_end = ALIGN_UP(multiboot_virt_addr + (uint64_t)mb.mb_data_end - (uint64_t)mb.mb_data_start, 0x1000);
		for (; multiboot_virt_addr < multiboot_virt_end; multiboot_virt_addr+=0x1000, multiboot_phys_addr+=0x1000) {
			entry_flags_t flags = {
				.writeable = 0,
				.user_accessible = 0,
				.write_through = 0,
				.cache_disabled = 0,
				.accessed = 0,
				.dirty = 0,
				.huge = 0,
				.global = 0,
				.no_execute = 1
			};
			
			map_page_to(multiboot_virt_addr, multiboot_phys_addr, flags, global_frame_allocator);
		}
		multiboot_virt_addr = ALIGN_UP(0xFFFFFF8040000000 + fb->height*fb->pitch, 0x1000) + (multiboot_addr - ALIGN_DOWN(multiboot_addr, 0x1000));

		// Map initramfs
		uint64_t initramfs_phys_addr = ALIGN_DOWN(boot_module->module_start, 0x1000);
		initramfs_virt_addr = ALIGN_UP(multiboot_virt_end, 0x1000); 
		uint64_t initramfs_virt_end = ALIGN_UP(initramfs_virt_addr + (uint64_t)boot_module->module_end - (uint64_t)boot_module->module_start, 0x1000);
		for (; initramfs_virt_addr < initramfs_virt_end; initramfs_virt_addr+=0x1000, initramfs_phys_addr+=0x1000) {
			entry_flags_t flags = {
				.writeable = 0,
				.user_accessible = 0,
				.write_through = 0,
				.cache_disabled = 0,
				.accessed = 0,
				.dirty = 0,
				.huge = 0,
				.global = 0,
				.no_execute = 1
			};

			map_page_to(initramfs_virt_addr, initramfs_phys_addr, flags, global_frame_allocator);
		}
		initramfs_virt_addr = ALIGN_UP(multiboot_virt_end, 0x1000) + (boot_module->module_start - ALIGN_DOWN(boot_module->module_start, 0x1000));

		// Map bitmap
		memory_bitmap = ALIGN_UP(initramfs_virt_end, 0x1000);
		printf("[    ] Allocating %u bytes for memory bitmap (%u frames) at %p\n", needed_bytes, needed_frames, memory_bitmap);
		for (uint64_t i = 0; i < needed_frames; ++i) {
			entry_flags_t flags = {
				.writeable = 1,
				.user_accessible = 0,
				.write_through = 0,
				.cache_disabled = 0,
				.accessed = 0,
				.dirty = 0,
				.huge = 0,
				.global = 1,
				.no_execute = 1
			};

			map_page(memory_bitmap + i*0x1000, flags, global_frame_allocator);
		}
		printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Allocated memory bitmap (ends at %p)\n", memory_bitmap + needed_frames*0x1000);
		
		mapper_ctx_end(ctx);
	}

	uint64_t old_p4_table_page;
	__asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_page));
	old_p4_table_page += 0xFFFFFF8000000000;
	__asm__ volatile("mov %0, %%cr3" : : "r"(new_p4_table));

	// Reload multiboot info from new address
	fprintf(stdserial, "Remapped multiboot to %p\n", multiboot_virt_addr);
	mb = multiboot::Data(multiboot_virt_addr);
	fb = mb.find_tag<multiboot::framebuffer_tag>(multiboot::tag_type::FRAMEBUFFER);
	bootloader = mb.find_tag<multiboot::bootloader_tag>(multiboot::tag_type::BOOTLOADER_NAME);
	cli = mb.find_tag<multiboot::cli_tag>(multiboot::tag_type::CLI);
	mmap = mb.find_tag<multiboot::memory_map_tag>(multiboot::tag_type::MEMORY_MAP);
	sections = mb.find_tag<multiboot::elf_sections_tag>(multiboot::tag_type::ELF_SECTIONS);
	boot_module = mb.find_tag<multiboot::boot_module_tag>(multiboot::tag_type::BOOT_MODULE);

	fprintf(stdserial, "Creating stack guard page at %lp\n", old_p4_table_page);
	unmap_page_no_free(old_p4_table_page);

	fprintf(stdout, "[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Reloaded page tables\n");

	uint64_t memory_bitmap_start = memory_bitmap;
	uint64_t memory_bitmap_end = memory_bitmap_start + needed_bytes;

	frame_main_alloc_state main_frame_allocator = {
		.vtable = frame_main_alloc_state_vtable,
		.bitmap_start = reinterpret_cast<uint64_t*>(memory_bitmap_start),
		.bitmap_end = reinterpret_cast<uint64_t*>(memory_bitmap_end)
	};
	printf("[    ] Initialising memory bitmap\n");
	for (uint64_t *i = reinterpret_cast<uint64_t*>(memory_bitmap_start); i < reinterpret_cast<uint64_t*>(memory_bitmap_end); ++i) {
		*i = 0;
	}
	for (uint64_t i = 0; i < init_alloc.next_alloc; i+=0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (uint64_t i = init_alloc.kernel_start; i < init_alloc.kernel_end; i+=0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (uint64_t i = init_alloc.multiboot_start; i < init_alloc.multiboot_end; i+=0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (multiboot::memory_map_entry& entry : *mmap) {
		if (entry.type != multiboot::memory_type::AVAILABLE) {
			for (uint64_t i = ALIGN_DOWN(entry.base_addr, 0x1000); i < ALIGN_UP(entry.base_addr + entry.length, 0x1000); i+=0x1000) {
				main_frame_allocator.set_bit(i);
			}
		}
	}
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory bitmap\n");

	global_frame_allocator = &main_frame_allocator.vtable;

	Stack double_fault_stack(0x1000);
	Stack kernel_interrupt_stack(0x1000);
	tss::TSS task_state_segment = tss::TSS();
	task_state_segment.interrupt_stack_table[0] = double_fault_stack.top;
	task_state_segment.privilege_stack_table[0] = kernel_interrupt_stack.top;
	
	uint8_t index = global_descriptor_table.add_tss_entry(gdt::tss_entry(reinterpret_cast<uint64_t>(&task_state_segment), sizeof(tss::TSS), 0));
	printf("TSS index at %u\n", index);
	task_state_segment.load(index);

	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded TSS\n");

	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory\n");
	printf("[    ] Initialising sbrk and heap\n");
	global_sbrk_state = sbrk_state_t {
		.kernel_end = memory_bitmap_end,
		.current_break = ALIGN_UP(memory_bitmap_end, 0x1000),
		.initialised = true
	};
	//init_heap();
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised sbrk and heap\n");

	Initramfs ramfs(initramfs_virt_addr, initramfs_virt_addr + (boot_module->module_end - boot_module->module_start));
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised initramfs\n");
	printf("Contents of initramfs/.placeholder:\n");
	void *data;
	size_t size = ramfs.locate_file("initramfs/.placeholder", &data);
	for (int i = 0; i < size; ++i) fputc(reinterpret_cast<uint8_t*>(data)[i], stdout);
	fputc('\n', stdout);

	printf("[    ] Initialising multitasking\n");
	auto kmain_task = threads::init_multitasking(old_p4_table_page + 8*0x1000, old_p4_table_page + 0x1000);

	auto test = threads::Task::new_kernel_task("test", (void(*)(uint64_t, uint64_t, uint64_t))test_task, 54, 12, 23);
	threads::scheduler.lock().add_task(test);
	threads::scheduler.lock().schedule();
	printf("back to kmain\n");
	threads::scheduler.lock().schedule();
	while(1);

	printf("Running tasks:\n");
	threads::scheduler.lock().print_tasks();

	while(1);
}
