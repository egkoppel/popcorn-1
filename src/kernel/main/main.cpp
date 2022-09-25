#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <termcolor.h>

#include "main.hpp"
#include "multiboot.hpp"
#include "../memory/memory.h"
#include "../memory/paging.h"
#include "../memory/stack.hpp"
#include "../memory/frame_bump_alloc.hpp"
#include "../memory/frame_main_alloc.hpp"
#include "../memory/kernelspace_map.hpp"
#include "../interrupts/interrupt_handlers.hpp"
#include "../interrupts/pit.hpp"
#include "../gdt/gdt.hpp"
#include "../gdt/tss.hpp"
#include "../userspace/initramfs.hpp"
#include "../threading/threading.hpp"
#include "../userspace/uinit.hpp"

#include <panic.h>

#define USER_ACCESS_FROM_KERNEL 0
#if USER_ACCESS_FROM_KERNEL == 1
#warning USER_ACCESS_FROM_KERNEL is enabled - THIS IS A TERRIBLE TERRIBLE TERRIBLE IDEA FOR SECURITY
#endif

extern "C" gdt::GDT global_descriptor_table = gdt::GDT();
extern "C" tss::TSS task_state_segment = tss::TSS();

extern "C" allocator_vtable *global_frame_allocator = nullptr;
extern "C" void switch_to_user_mode(void);

extern "C" void kmain(uint32_t multiboot_magic, uint32_t multiboot_addr) {
	if (multiboot_magic == 0x36d76289) {
		printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Mulitboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		printf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Mulitboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	multiboot::Data mb(multiboot_addr);

	auto *fb = mb.find_tag<multiboot::framebuffer_tag>(multiboot::tag_type::FRAMEBUFFER);
	auto *bootloader = mb.find_tag<multiboot::bootloader_tag>(multiboot::tag_type::BOOTLOADER_NAME);
	auto *cli = mb.find_tag<multiboot::cli_tag>(multiboot::tag_type::CLI);
	auto *mmap = mb.find_tag<multiboot::memory_map_tag>(multiboot::tag_type::MEMORY_MAP);
	auto *sections = mb.find_tag<multiboot::elf_sections_tag>(multiboot::tag_type::ELF_SECTIONS);
	auto *boot_module = mb.find_tag<multiboot::boot_module_tag>(multiboot::tag_type::BOOT_MODULE);

	if (!mmap) panic("No memory map tag found");
	if (bootloader) printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Booted by %s\n", bootloader->get_name());
	if (!boot_module) panic("No initramfs found");
	if (strcmp(boot_module->get_name(), "initramfs") != 0) panic("No initramfs found");

	init_idt();
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded IDT\n");

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
	printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Detected %d MiB of available memory (%d MiB total):\n",
	       available_ram / (1024 * 1024), total_ram / (1024 * 1024));

	for (multiboot::memory_map_entry entry : *mmap) {
		printf("\t%lp - %lp (%s)\n", entry.base_addr, entry.base_addr + entry.length,
		       entry.type == multiboot::memory_type::AVAILABLE ? "AVAILABLE" : "RESERVED");
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

	// ******************************* BEGIN PAGE TABLE REMAPPING *******************************
	uint64_t new_p4_table = create_p4_table(&init_alloc.vtable);
	mapper_ctx_t ctx = mapper_ctx_begin(new_p4_table, global_frame_allocator);

	// Map the kernel with section flags
	for (multiboot::elf_sections_entry i : *sections) {
		if ((i.type != SHT_NULL) && (i.flags & SHF_ALLOC) != 0) {
			auto name = reinterpret_cast<char *>(sections->find_strtab()->addr + i.name_index);
			auto is_userspace = strncmp(name, ".userspace", 10) == 0;
			printf("%d\n", is_userspace || USER_ACCESS_FROM_KERNEL);

			for (uint64_t phys_addr = i.addr - 0xFFFFFF8000000000;
			     phys_addr < ALIGN_UP(i.addr - 0xFFFFFF8000000000 + i.size, 0x1000); phys_addr += 0x1000) {
				entry_flags_t flags = {
						.writeable = static_cast<bool>(i.flags & SHF_WRITE),
						.user_accessible = is_userspace || USER_ACCESS_FROM_KERNEL,
						.write_through = false,
						.cache_disabled = false,
						.accessed = false,
						.dirty = false,
						.huge = false,
						.global = true,
						.no_execute = !(i.flags & SHF_EXECINSTR)
				};

				fprintf(stdserial, "Kexe - Mapping %p -> %p\n", phys_addr + 0xFFFFFF8000000000, phys_addr);
				map_page_to(phys_addr + 0xFFFFFF8000000000, phys_addr, flags, global_frame_allocator);
			}
		}
	}

	auto kernel_space_mapper = KernelspaceMapper(0xFFFFFF8040000000);

	fprintf(stdserial, "Map framebuffer\n");
	entry_flags_t framebuffer_flags = {
			.writeable = true,
			.user_accessible = USER_ACCESS_FROM_KERNEL,
			.write_through = false,
			.cache_disabled = false,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = true,
			.no_execute = true
	};
	kernel_space_mapper.map_to(fb->addr, fb->height * fb->pitch, framebuffer_flags, global_frame_allocator);

	fprintf(stdserial, "Map multiboot\n");
	entry_flags_t multiboot_flags = {
			.writeable = false,
			.user_accessible = USER_ACCESS_FROM_KERNEL,
			.write_through = false,
			.cache_disabled = false,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = false,
			.no_execute = true
	};
	uint64_t multiboot_address = kernel_space_mapper.map_to(multiboot_addr,
	                                                        (uint64_t)mb.mb_data_end - (uint64_t)mb.mb_data_start,
	                                                        multiboot_flags, global_frame_allocator);

	fprintf(stdserial, "Map initramfs\n");
	entry_flags_t initramfs_flags = {
			.writeable = false,
			.user_accessible = true,
			.write_through = false,
			.cache_disabled = false,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = false,
			.no_execute = true
	};
	uint64_t initramfs_address = kernel_space_mapper.map_to(boot_module->module_start,
	                                                        (uint64_t)boot_module->module_end -
	                                                        (uint64_t)boot_module->module_start, initramfs_flags,
	                                                        global_frame_allocator);

	fprintf(stdserial, "Map bitmap\n");
	uint64_t bitmap_needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);

	printf("[    ] Allocating %u bytes for memory bitmap\n", bitmap_needed_bytes);
	entry_flags_t bitmap_flags = {
			.writeable = true,
			.user_accessible = USER_ACCESS_FROM_KERNEL,
			.write_through = false,
			.cache_disabled = false,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = true,
			.no_execute = true
	};
	uint64_t memory_bitmap_address = kernel_space_mapper.map(bitmap_needed_bytes, bitmap_flags,
	                                                         global_frame_allocator);
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Allocated memory bitmap at %p\n", memory_bitmap_address);

	mapper_ctx_end(ctx);
	// ******************************* END PAGE TABLE REMAPPING *******************************

	uint64_t old_p4_table_page;
	__asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_page));
	old_p4_table_page += 0xFFFFFF8000000000;
	__asm__ volatile("mov %0, %%cr3" : : "r"(new_p4_table));

	// Reload multiboot info from new address
	fprintf(stdserial, "Remapped multiboot to %p\n", multiboot_address);
	mb = multiboot::Data(multiboot_address);
	fb = mb.find_tag<multiboot::framebuffer_tag>(multiboot::tag_type::FRAMEBUFFER);
	bootloader = mb.find_tag<multiboot::bootloader_tag>(multiboot::tag_type::BOOTLOADER_NAME);
	cli = mb.find_tag<multiboot::cli_tag>(multiboot::tag_type::CLI);
	mmap = mb.find_tag<multiboot::memory_map_tag>(multiboot::tag_type::MEMORY_MAP);
	sections = mb.find_tag<multiboot::elf_sections_tag>(multiboot::tag_type::ELF_SECTIONS);
	boot_module = mb.find_tag<multiboot::boot_module_tag>(multiboot::tag_type::BOOT_MODULE);

	fprintf(stdserial, "Creating stack guard page at %lp\n", old_p4_table_page);
	unmap_page_no_free(old_p4_table_page);

	fprintf(stdout, "[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Reloaded page tables\n");

	uint64_t memory_bitmap_start = memory_bitmap_address;
	uint64_t memory_bitmap_end = memory_bitmap_start + bitmap_needed_bytes;

	frame_main_alloc_state main_frame_allocator = {
			.vtable = frame_main_alloc_state_vtable,
			.bitmap_start = reinterpret_cast<uint64_t *>(memory_bitmap_start),
			.bitmap_end = reinterpret_cast<uint64_t *>(memory_bitmap_end)
	};
	printf("[    ] Initialising memory bitmap\n");
	for (auto *i = reinterpret_cast<uint64_t *>(memory_bitmap_start);
	     i < reinterpret_cast<uint64_t *>(memory_bitmap_end); ++i) {
		*i = 0;
	}
	printf("init_alloc.next_alloc: %p\n", init_alloc.next_alloc);
	for (uint64_t i = 0; i < init_alloc.next_alloc; i += 0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (uint64_t i = init_alloc.kernel_start; i < init_alloc.kernel_end; i += 0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (uint64_t i = init_alloc.multiboot_start; i < init_alloc.multiboot_end; i += 0x1000) {
		main_frame_allocator.set_bit(i);
	}
	for (multiboot::memory_map_entry& entry : *mmap) {
		if (entry.type != multiboot::memory_type::AVAILABLE) {
			for (uint64_t i = ALIGN_DOWN(entry.base_addr, 0x1000);
			     i < ALIGN_UP(entry.base_addr + entry.length, 0x1000); i += 0x1000) {
				main_frame_allocator.set_bit(i);
			}
		}
	}
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory bitmap\n");

	global_frame_allocator = &main_frame_allocator.vtable;

	Stack double_fault_stack(0x1000);
	task_state_segment.interrupt_stack_table[0] = double_fault_stack.top;

	uint8_t index = global_descriptor_table.add_tss_entry(
			gdt::tss_entry(reinterpret_cast<uint64_t>(&task_state_segment), sizeof(tss::TSS), 0));
	printf("TSS index at %u\n", index);
	tss::TSS::load(index);
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded TSS\n");

	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory\n");
	printf("[    ] Initialising sbrk and heap\n");
	global_sbrk_state = sbrk_state_t{
			.kernel_end = memory_bitmap_end,
			.current_break = ALIGN_UP(memory_bitmap_end, 0x1000),
			.initialised = true
	};
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised sbrk and heap\n");

	Initramfs ramfs(initramfs_address, initramfs_address + (boot_module->module_end - boot_module->module_start));

	printf("[    ] Initialising multitasking\n");
	auto ktask = threads::Scheduler::init_multitasking(old_p4_table_page + 0x1000, old_p4_table_page + 8 * 0x1000);
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised multitasking\n");

	// Switch to userspace, and stack switch, and call init
	auto userspace_stack_top = ktask->get_code_stack().top;
	// place init function at top of stack
	*(reinterpret_cast<uint64_t *>(userspace_stack_top) - 1) = reinterpret_cast<uint64_t>(uinit);
	// place userpace switch
	*(reinterpret_cast<uint64_t *>(userspace_stack_top) - 2) = reinterpret_cast<uint64_t>(switch_to_user_mode);
	auto new_stack_ptr = userspace_stack_top - 2 * 8;

	__asm__ volatile("xor %%rbp, %%rbp; mov %%rax, %%rsp; ret;" : : "a"(new_stack_ptr));

	panic("");
}
