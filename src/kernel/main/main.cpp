/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include "../userspace/fsd.hpp"
#include "../smp/core_local.hpp"
#include "../acpi/acpi.hpp"
#include "../acpi/apic.hpp"
#include "../acpi/lapic.hpp"

#include <panic.h>

#define USER_ACCESS_FROM_KERNEL 0
#if USER_ACCESS_FROM_KERNEL == 1
#warning USER_ACCESS_FROM_KERNEL is enabled - THIS IS A TERRIBLE TERRIBLE TERRIBLE IDEA FOR SECURITY
#endif

extern "C" gdt::GDT global_descriptor_table = gdt::GDT();
extern "C" tss::TSS task_state_segment = tss::TSS();

extern "C" allocator_vtable *global_frame_allocator = nullptr;
extern "C" void switch_to_user_mode(void);
extern "C" volatile uint8_t ap_running_count;
volatile uint8_t *real_ap_running_count = &ap_running_count + 0xFFFFFF8000000000;
extern "C" volatile uint8_t ap_wait_flag;
volatile uint8_t *real_ap_wait_flag = &ap_wait_flag + 0xFFFFFF8000000000;

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

			auto name = reinterpret_cast<char *>(sections->find_strtab()->addr + i.name_index);
			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset = 0;
			if (!is_ap_bootstrap) offset = 0xFFFFFF8000000000;

			if (i.addr - offset < kernel_min) kernel_min = i.addr - offset;
			if (i.addr - offset + i.size > kernel_max) kernel_max = i.addr - offset + i.size;
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

			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset = 0;
			if (!is_ap_bootstrap) offset = 0xFFFFFF8000000000;

			for (uint64_t phys_addr = i.addr - offset;
			     phys_addr < ALIGN_UP(i.addr - offset + i.size, 0x1000); phys_addr += 0x1000) {
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
	auto rdsp_version = 0;
	auto rsdp_tag = mb.find_tag<multiboot::rsdp_tag>(multiboot::tag_type::RSDT_V1);
	if (!rsdp_tag) {
		rsdp_tag = mb.find_tag<multiboot::rsdp_tag>(multiboot::tag_type::RSDT_V2);
		rdsp_version = 2;
	} else rdsp_version = 1;

	fprintf(stdserial, "Found rdsp version %d\n", rdsp_version);
	if (strncmp(reinterpret_cast<const char *>(&rsdp_tag->signature), "RSD PTR ", 8) != 0) {
		panic("RSDP signature wrong");
	}

	char oem_str_buf[7] = {0};
	memcpy(oem_str_buf, &rsdp_tag->oem_id, 6);
	fprintf(stdserial, "OEM is %s\n", oem_str_buf);

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
			.kernel_end = kernel_space_mapper.get_next_addr() + 1024 * 1024, // Leave 1MiB buffer for any mmio mapping
			.current_break = ALIGN_UP(kernel_space_mapper.get_next_addr() + 1024 * 1024, 0x1000),
			.initialised = true
	};
	kernel_space_mapper.set_max_addr(global_sbrk_state.kernel_end); // Don't let mapper map into the heap
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised sbrk and heap\n");

	Initramfs ramfs(initramfs_address, initramfs_address + (boot_module->module_end - boot_module->module_start));

	printf("[    ] Initialising multitasking\n");
	auto ktask = threads::init_multitasking(old_p4_table_page + 0x1000, old_p4_table_page + 8 * 0x1000);
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised multitasking\n");

	printf("[    ] Finding cores\n");
	entry_flags_t rsdt_flags = {};
	auto rsdt = (RSDT *)kernel_space_mapper.map_to(rdsp_version == 2 ? (uint32_t)rsdp_tag->xsdt_addr : rsdp_tag->rsdt_addr, 0x1000, rsdt_flags, global_frame_allocator);

	char bufbuf[5] = {0};
	memcpy(bufbuf, rsdt->header.signature, 4);
	fprintf(stdserial, "RSDT signature is %s\n", bufbuf);
	assert_msg(strcmp(bufbuf, "RSDT") == 0, "RSDT sig wrong");
	auto madt = (MADT *)rsdt->find_sdt("APIC", kernel_space_mapper, global_frame_allocator);
	fprintf(stdserial, "Found APIC at %p, with entries:\n", madt);

	uint64_t lapic_addr = madt->lapic_address;
	uint8_t processor_ids[256] = {0};
	uint64_t ioapic_addr = 0;
	uint64_t core_count = 0;

	for (auto& madt_entry : *madt) {
		switch (madt_entry.type) {
			case madt_entry_types::CPU_LAPIC: {
				auto lapic = (madt_entry_lapic *)&madt_entry;
				if (lapic->flags & 1) {
					processor_ids[core_count++] = lapic->processor_id;
				}
				break;
			}
			case madt_entry_types::IO_APIC: {
				auto ioapic = (madt_entry_ioapic *)&madt_entry;
				ioapic_addr = ioapic->io_apic_addr;
				break;
			}
			case madt_entry_types::LAPIC_ADDR: {
				auto lapic_addr_entry = (madt_entry_lapic_addr *)&madt_entry;
				lapic_addr = lapic_addr_entry->lapic_addr;
				break;
			}
			default: break;
		}
	}

	fprintf(stdserial, "Found %d cores, IOAPIC addr %p, LAPIC addr %p, processor ids:\n", core_count, ioapic_addr, lapic_addr);
	for (int i = 0; i < core_count; i++)
		fprintf(stdserial, " %d\n", processor_ids[i]);
	entry_flags_t lapic_flags = {
			.writeable = true,
			.user_accessible = false,
			.write_through = true,
			.cache_disabled = true,
			.accessed = false,
			.dirty = false,
			.huge = false,
			.global = false,
			.no_execute = true,
	};
	uint32_t msr_low;
	__asm__ volatile("mov $0x1b, %%rcx; rdmsr;" : "=a"(msr_low)::"rcx", "rdx");
	fprintf(stdserial, "apic base msr is %b\n", msr_low);

	auto lapic_base = (volatile uint32_t *)kernel_space_mapper.map_to(lapic_addr, 0x3f4, lapic_flags, global_frame_allocator);
	fprintf(stdserial, "lapic spiv at %p\n", &lapic_base[lapic_registers::SPURIOUS_INTERRUPT_VECTOR]);
	lapic_base[lapic_registers::SPURIOUS_INTERRUPT_VECTOR] = 0x1ff;

	uint8_t bsp_core_id;
	__asm__ volatile("mov $1, %%eax; cpuid; shrl $24, %%ebx;": "=b"(bsp_core_id) : : "eax", "ecx", "edx");
	fprintf(stdserial, "BSP id is %d\n", bsp_core_id);
	fprintf(stdserial, "ap running count is %d\n", *real_ap_running_count);

	for (auto core_id : processor_ids) {
		if (core_id == bsp_core_id) continue;

		fprintf(stdserial, "Booting core %d\n", core_id);

		// Send INIT IPI
		lapic_base[lapic_registers::ERROR_STATUS] = 0;
		lapic_base[lapic_registers::ICR_HIGH] = (lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
		lapic_base[lapic_registers::ICR_LOW] = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x00C500;
		do {
			__asm__ volatile("pause":: : "memory");
		} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12)); // Wait for delivered bit to clear

		// Deassert INIT IPI
		lapic_base[lapic_registers::ICR_HIGH] = (lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
		lapic_base[lapic_registers::ICR_LOW] = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x008500;
		do {
			__asm__ volatile("pause":: : "memory");
		} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12)); // Wait for delivered bit to clear

		get_local_data()->scheduler.sleep(10);

		for (int i = 0; i < 2; i++) {
			// Send SIPI
			lapic_base[lapic_registers::ERROR_STATUS] = 0;
			lapic_base[lapic_registers::ICR_HIGH] = (lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
			lapic_base[lapic_registers::ICR_LOW] = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x608;
			get_local_data()->scheduler.sleep_ns(200'000);
			do {
				__asm__ volatile("pause":: : "memory");
			} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12)); // Wait for delivered bit to clear
		}
	}

	*real_ap_wait_flag = 1;
	get_local_data()->scheduler.sleep(5);
	fprintf(stdserial, "ap running count is %d\n", *real_ap_running_count);

	// Switch to userspace, and stack switch, and call init
	auto userspace_stack_top = ktask->get_code_stack().top;
	// place init function at top of stack
	*(reinterpret_cast<uint64_t *>(userspace_stack_top) - 1) = reinterpret_cast<uint64_t>(fsd_main);
	// place userpace switch
	*(reinterpret_cast<uint64_t *>(userspace_stack_top) - 2) = reinterpret_cast<uint64_t>(switch_to_user_mode);
	auto new_stack_ptr = userspace_stack_top - 2 * 8;

	__asm__ volatile("xor %%rbp, %%rbp; mov %%rax, %%rsp; ret;" : : "a"(new_stack_ptr));

	panic("");
}
