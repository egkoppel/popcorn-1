/*
 * Copyright (c) 2022 Eliyahu Gluschove-Koppel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.hpp"

#include <acpi/acpi.hpp>
#include <acpi/apic.hpp>
#include <acpi/lapic.hpp>
#include <arch/amd64/macros.hpp>
#include <arch/constants.hpp>
#include <arch/hal.hpp>
#include <arch/initialisation.hpp>
#include <arch/interrupts.hpp>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <interrupt_handlers/double_fault.hpp>
#include <interrupt_handlers/page_fault.hpp>
#include <log.hpp>
#include <memory/memory_map.hpp>
#include <memory/paging.hpp>
#include <memory/physical_allocators/bitmap_allocator.hpp>
#include <memory/physical_allocators/monotonic_allocator.hpp>
#include <memory/stack.hpp>
#include <memory/types.hpp>
#include <memory/virtual_allocators/monotonic_allocator.hpp>
#include <memory/vm_map.hpp>
#include <multiboot/boot_module.hpp>
#include <multiboot/bootloader.hpp>
#include <multiboot/cli.hpp>
#include <multiboot/elf_sections.hpp>
#include <multiboot/framebuffer.hpp>
#include <multiboot/memory_map.hpp>
#include <multiboot/multiboot.hpp>
#include <multiboot/rsdp.hpp>
#include <panic.h>
#include <smp/core_local.hpp>
#include <termcolor.h>
#include <threading/task.hpp>
#include <tuple>
#include <userspace/fsd.hpp>
#include <userspace/initramfs.hpp>
#include <userspace/uinit.hpp>

#define USER_ACCESS_FROM_KERNEL 0
#if USER_ACCESS_FROM_KERNEL == 1
	#warning USER_ACCESS_FROM_KERNEL is enabled - THIS IS A TERRIBLE TERRIBLE TERRIBLE IDEA FOR SECURITY
#endif

kernel_allocators_t allocators = {};

extern "C" volatile u8 ap_running_count;
volatile u8 *real_ap_running_count = &ap_running_count + memory::constants::kexe_start;
extern "C" volatile u8 ap_wait_flag;
volatile u8 *real_ap_wait_flag = &ap_wait_flag + memory::constants::kexe_start;
extern "C" char *FRAMEBUFFER;
extern "C" u8 initial_mem_map_start;
memory::paddr_t real_initial_mem_map_start{
		.address = reinterpret_cast<usize>(&initial_mem_map_start - memory::constants::kexe_start)};

using namespace memory;

/**
 * @brief Kernel main entrypoint
 * @param multiboot_magic The multiboot2 magic number
 * @param multiboot_addr 32 bit pointer to the physical address the multiboot2 info struct is located at
 *
 * Expected system state:
 *  - Long mode enabled
 *  - Paging enabled
 *  - Interrupts disabled
 *  - Running from higher half
 *
 * Expected paging state:
 *  - Kernel mapped to higher half starting at `memory::constants::kexe_start`
 *  - First gigabyte of physical memory contiguously mapped starting at `memory::constants::page_offset_start`
 */
extern "C" void kmain(u32 multiboot_magic, paddr32_t multiboot_addr) noexcept try {
	if (multiboot_magic == 0x36d76289) {
		printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Multiboot magic: 0x%x (correct)\n", multiboot_magic);
	} else {
		printf("[" TERMCOLOR_RED "FAIL" TERMCOLOR_RESET "] Multiboot magic: 0x%x (incorrect)\n", multiboot_magic);
	}

	// FIXME: why the flip does this not work
	paddr_t _a        = multiboot_addr;
	decltype(auto) mb = *static_cast<const multiboot::Data *>(_a.virtualise());

	auto bootloader = mb.find_tag<multiboot::tags::Bootloader>(multiboot::TagType::BOOTLOADER_NAME);
	//auto cli_ = mb.find_tag<multiboot::tags::Cli>(multiboot::TagType::CLI);

	if (bootloader) printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Booted by %s\n", bootloader->name());

	auto fb = mb.find_tag<multiboot::tags::Framebuffer>(multiboot::TagType::FRAMEBUFFER).value();
	//auto cli = mb.find_tag<multiboot::tags::Cli>(multiboot::TagType::CLI).value();
	auto mmap        = mb.find_tag<multiboot::tags::MemoryMap>(multiboot::TagType::MEMORY_MAP).value();
	auto sections    = mb.find_tag<multiboot::tags::ElfSections>(multiboot::TagType::ELF_SECTIONS).value();
	auto boot_module = mb.find_tag<multiboot::tags::BootModule>(multiboot::TagType::BOOT_MODULE).value();

	if (strcmp(boot_module->name(), "initramfs") != 0) panic("No initramfs found");

	arch::arch_specific_early_init();
	arch::load_interrupt_handler(arch::InterruptVectors::PAGE_FAULT, false, 0, interrupt_handlers::page_fault);
	arch::load_interrupt_handler(arch::InterruptVectors::DOUBLE_FAULT, false, 1, interrupt_handlers::double_fault);
	arch::load_interrupt_handler(arch::InterruptVectors::CORE_TIMER, false, 0, [](arch::interrupt_info_t *) noexcept {
		threads::local_scheduler->irq_fired();
	});
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded IDT\n");

	printf("[    ] Initialising memory\n");

	auto kernel_max = 0_pa;
	auto kernel_min = 0xffff'ffff'ffff'ffff_pa;
	for (auto& i : *sections) {
		if ((i.type() != decltype(i.type())::SHT_NULL)
		    && (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_ALLOC) != 0) {
			//i.print();

			auto name            = i.name(*sections);
			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset      = 0;
			if (!is_ap_bootstrap) offset = memory::constants::kexe_start;

			if (i.start() - offset < kernel_min) kernel_min = i.start() - offset;
			if (i.end() - offset > kernel_max) kernel_max = i.end() - offset;
		}
	}
	printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Kernel executable: %lp -> %lp\n", kernel_min, kernel_max);

	uint64_t available_ram = 0;
	uint64_t total_ram     = 0;
	for (auto& i : *mmap) {
		if (i.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
			available_ram += i.get_size();

			if (i.get_end_address().address > total_ram) total_ram = i.get_end_address().address;
		}
	}
	printf("[" TERMCOLOR_CYAN "INFO" TERMCOLOR_RESET "] Detected %d MiB of available memory (%d MiB total):\n",
	       available_ram / (1024 * 1024),
	       total_ram / (1024 * 1024));

	for (auto& entry : *mmap) {
		printf("\t%lp - %lp (%s)\n",
		       entry.get_start_address(),
		       entry.get_end_address(),
		       entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE ? "AVAILABLE" : "RESERVED");
	}

	LOG(Log::DEBUG,
	    "Detected %d MiB of available memory (%d MiB total)",
	    available_ram / (1024 * 1024),
	    total_ram / (1024 * 1024));
	//}

	// Basic allocator that allocates between 1MiB and 1GiB (where page_offset area and mem_map initially end)
	auto kernel_monotonic_frame_allocator = memory::physical_allocators::MonotonicAllocator(0x100000_pa,
	                                                                                        0x40000000_pa,
	                                                                                        kernel_min,
	                                                                                        kernel_max,
	                                                                                        mb.begin().devirtualise(),
	                                                                                        mb.end().devirtualise(),
	                                                                                        mmap);

	// ******************************* BEGIN PAGE TABLE REMAPPING *******************************
	auto& new_p4_table = paging::init_kas(kernel_monotonic_frame_allocator);

	// Map the kernel with section flags
	for (auto& i : *sections) {
		if ((i.type() != multiboot::tags::ElfSections::Entry::Type::SHT_NULL)
		    && (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_ALLOC) != 0) {
			auto name         = i.name(*sections);
			auto is_userspace = strncmp(name, ".userspace", 10) == 0;

			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset      = 0;
			if (!is_ap_bootstrap) offset = memory::constants::kexe_start;

			for (auto phys_frame = aligned<paddr_t>::aligned_down(i.start() - offset);
			     phys_frame < aligned<paddr_t>::aligned_up(i.end() - offset);
			     phys_frame++) {
				using enum memory::paging::PageTableFlags;
				paging::PageTableFlags flags;
				if (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_WRITE) flags = flags | WRITEABLE;
				if (is_userspace || USER_ACCESS_FROM_KERNEL) flags = flags | USER;
				flags = flags | GLOBAL;
				if (!(i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_EXECINSTR))
					flags = flags | NO_EXECUTE;

				fprintf(stdserial,
				        "Kexe - Mapping %p -> %p\n",
				        phys_frame.address + memory::constants::kexe_start,
				        phys_frame.address);
				new_p4_table.map_page_to(vaddr_t{.address = phys_frame.address.address + memory::constants::kexe_start},
				                         phys_frame.frame(),
				                         flags);
			}
		}
	}

	auto kernel_virt_allocator = memory::virtual_allocators::MonotonicAllocator(
			vaddr_t{.address = memory::constants::kernel_page_allocator_start},
			vaddr_t{.address = memory::constants::kernel_page_allocator_end});

	while (true) __asm__ volatile("nop");
#if 0
	fprintf(stdserial, "Map framebuffer\n");
	memory::paging::PageTableEntry::flags_t framebuffer_flags = 0;
	framebuffer_flags |= memory::paging::PageTableEntry::flags::WRITEABLE;
	framebuffer_flags |= memory::paging::PageTableEntry::flags::GLOBAL;
	framebuffer_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;
	framebuffer_flags |= memory::paging::PageTableEntry::flags::IMPL_CACHE_DISABLE;
	framebuffer_flags |= memory::paging::PageTableEntry::flags::IMPL_CACHE_WRITETHROUGH;
	if (USER_ACCESS_FROM_KERNEL) framebuffer_flags |= memory::paging::PageTableEntry::flags::USER;

	VirtualAddress framebuffer_address = MemoryMapper::new_address_map(fb->begin(),
	                                                                   fb->size(),
	                                                                   framebuffer_flags,
	                                                                   kernel_virt_allocator,
	                                                                   allocators.general(),
	                                                                   new_p4_table);

	/*fprintf(stdserial, "Map multiboot\n");
	memory::paging::PageTableEntry::flags_t multiboot_flags = 0;
	multiboot_flags |= memory::paging::PageTableEntry::flags::GLOBAL;
	multiboot_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;
	if (USER_ACCESS_FROM_KERNEL) multiboot_flags |= memory::paging::PageTableEntry::flags::USER;

	VirtualAddress multiboot_address = MemoryMapper::new_address_map(VirtualAddress(multiboot_addr),
	                                                                 mb->size(),
	                                                                 multiboot_flags,
	                                                                 kernel_virt_allocator,
	                                                                 allocators.general());*/

	/*fprintf(stdserial, "Map initramfs\n");
	memory::paging::PageTableEntry::flags_t initramfs_flags = 0;
	initramfs_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;
	initramfs_flags |= memory::paging::PageTableEntry::flags::USER;

	VirtualAddress initramfs_address = MemoryMapper::new_address_map(boot_module->begin(),
	                                                                 boot_module->module_size(),
	                                                                 initramfs_flags,
	                                                                 kernel_virt_allocator,
	                                                                 allocators.general(),
	                                                                 new_p4_table);*/

	fprintf(stdserial, "Map bitmap\n");
	uint64_t bitmap_needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);

	printf("[    ] Allocating %u bytes for memory bitmap\n", bitmap_needed_bytes);
	memory::paging::PageTableEntry::flags_t bitmap_flags = 0;
	bitmap_flags |= memory::paging::PageTableEntry::flags::WRITEABLE;
	bitmap_flags |= memory::paging::PageTableEntry::flags::GLOBAL;
	bitmap_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;
	if (USER_ACCESS_FROM_KERNEL) bitmap_flags |= memory::paging::PageTableEntry::flags::USER;

	VirtualAddress memory_bitmap_address = MemoryMapper::new_anonymous_map(bitmap_needed_bytes,
	                                                                       bitmap_flags,
	                                                                       kernel_virt_allocator,
	                                                                       allocators.general(),
	                                                                       allocators.general(),
	                                                                       new_p4_table)
	                                               .begin();

	fprintf(stdserial, "Map all the memory\n");
	memory::paging::PageTableEntry::flags_t all_mem_flags = 0;
	all_mem_flags |= memory::paging::PageTableEntry::flags::WRITEABLE;
	all_mem_flags |= memory::paging::PageTableEntry::flags::GLOBAL;
	all_mem_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;
	if (USER_ACCESS_FROM_KERNEL) all_mem_flags |= memory::paging::PageTableEntry::flags::USER;

	for (Frame f = Frame::from_address(0_pa); f.begin().address() < total_ram; f++) {
		new_p4_table.map_page_to(
				Page::from_address(VirtualAddress(f.begin().address() + memory::constants::page_offset_start)),
				f,
				all_mem_flags,
				allocators.general());
	}

	// ******************************* END PAGE TABLE REMAPPING *******************************

	/* TODO: Check if needs address offset like old code
	 * PhysicalAddress old_p4_table_frame;
	 * __asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_frame));
	 * Page old_p4_table_page = Page::from_address(static_cast<VirtualAddress>(old_p4_table_frame + 0xFFFF800000000000));
	 * __asm__ volatile("mov %0, %%cr3" : : "r"(new_p4_table));
	 */
	fprintf(stdserial, "************\n");
	new_p4_table.print_to(stdserial);
	paging::AddressSpace old_p4_table = new_p4_table.make_active();
	FRAMEBUFFER                       = static_cast<char *>(framebuffer_address);

	auto rdsp_version = 0;
	auto rsdp_tag_    = mb->find_tag<multiboot::tags::Rsdp>(multiboot::TagType::RSDT_V1);
	if (!rsdp_tag_) {
		rsdp_tag_    = mb->find_tag<multiboot::tags::Rsdp>(multiboot::TagType::RSDT_V2);
		rdsp_version = 2;
	} else rdsp_version = 1;
	auto rsdp_tag = rsdp_tag_.value();

	fprintf(stdserial, "Found rdsp version %d\n", rdsp_version);
	if (strncmp(reinterpret_cast<const char *>(&rsdp_tag->signature), "RSD PTR ", 8) != 0) {
		panic("RSDP signature wrong");
	}

	char oem_str_buf[7] = {0};
	memcpy(oem_str_buf, &rsdp_tag->oem_id, 6);
	fprintf(stdserial, "OEM is %s\n", oem_str_buf);

	// TODO: Fix this
	/*fprintf(stdserial, "Creating stack guard page at %lp\n", old_p4_table_page);
	unmap_page_no_free(old_p4_table_page);*/

	fprintf(stdout, "[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Reloaded page tables\n");

	printf("[    ] Initialising memory bitmap\n");
	auto memory_bitmap_start  = static_cast<uint64_t *>(memory_bitmap_address);
	auto memory_bitmap_end    = ADD_BYTES(memory_bitmap_start, bitmap_needed_bytes);
	auto main_frame_allocator = memory::physical_allocators::BitmapAllocator::from(
			Frame::from_address(0x100000_pa),
			dynamic_cast<memory::physical_allocators::MonotonicAllocator&>(allocators.general()),
			memory_bitmap_start,
			memory_bitmap_end);
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory bitmap\n");

	init_sbrk();

	allocators.general_frame_allocator_ =
			std::unique_ptr(new physical_allocators::BitmapAllocator(std::move(main_frame_allocator)));

	KStack double_fault_stack = KStack::new_stack(Page::size * 2, kernel_virt_allocator, allocators.general());
	arch::load_backup_stack(1, std::move(double_fault_stack));

	/*uint8_t index = global_descriptor_table.add_tss_entry(
			gdt::tss_entry(reinterpret_cast<uint64_t>(&task_state_segment), sizeof(tss::TSS), 0));
	printf("TSS index at %u\n", index);
	tss::TSS::load(index);*/
	//printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded TSS\n");

	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised memory\n");
	vm_map_init();

	while (true) hal::halt();

	printf("[    ] Initialising multitasking\n");
	auto ktask =
			nullptr; /*threads::Task::init_multitasking(old_p4_table_page + 0x1000, old_p4_table_page + 8 * 0x1000);*/
	printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Initialised multitasking\n");

	printf("[    ] Finding cores\n");
	memory::paging::PageTableEntry::flags_t rsdt_flags = 0;

	auto rsdt = acpi::RootSystemDescriptionTableReader(
			*static_cast<acpi::RootSystemDescriptionTable *>(MemoryMapper::new_address_map(
					PhysicalAddress(rdsp_version == 2 ? rsdp_tag->xsdt_addr : rsdp_tag->rsdt_addr),
					Page::size,
					rsdt_flags,
					kernel_virt_allocator,
					allocators.general())));

	auto madt = (acpi::Madt *)rsdt.find_sdt("APIC", kernel_virt_allocator, allocators.general()).value();
	fprintf(stdserial, "Found APIC at %p, with entries:\n", madt);

	PhysicalAddress lapic_addr = madt->lapic();
	uint8_t processor_ids[256] = {0};
	uint64_t ioapic_addr       = 0;
	uint64_t core_count        = 0;

	for (auto& madt_entry : *madt) {
		switch (madt_entry.type()) {
			case acpi::madt_entry_types::CPU_LAPIC: {
				auto lapic = (acpi::madt_entry_lapic *)&madt_entry;
				if (lapic->flags & 1) { processor_ids[core_count++] = lapic->processor_id; }
				break;
			}
			case acpi::madt_entry_types::IO_APIC: {
				auto ioapic = (acpi::madt_entry_ioapic *)&madt_entry;
				ioapic_addr = ioapic->io_apic_addr;
				break;
			}
			case acpi::madt_entry_types::LAPIC_ADDR: {
				auto lapic_addr_entry = (acpi::madt_entry_lapic_addr *)&madt_entry;
				lapic_addr            = lapic_addr_entry->lapic_addr;
				break;
			}
			default: break;
		}
	}

	fprintf(stdserial,
	        "Found %d cores, IOAPIC addr %p, LAPIC addr %p, processor ids:\n",
	        core_count,
	        ioapic_addr,
	        lapic_addr);
	for (uint64_t i = 0; i < core_count; i++) fprintf(stdserial, " %d\n", processor_ids[i]);
	memory::paging::PageTableEntry::flags_t lapic_flags = 0;
	lapic_flags |= memory::paging::PageTableEntry::flags::WRITEABLE;
	lapic_flags |= memory::paging::PageTableEntry::flags::IMPL_CACHE_WRITETHROUGH;
	lapic_flags |= memory::paging::PageTableEntry::flags::IMPL_CACHE_DISABLE;
	lapic_flags |= memory::paging::PageTableEntry::flags::GLOBAL;
	lapic_flags |= memory::paging::PageTableEntry::flags::NO_EXECUTE;

	uint32_t msr_low;
	__asm__ volatile("mov $0x1b, %%rcx; rdmsr;" : "=a"(msr_low)::"rcx", "rdx");
	fprintf(stdserial, "apic base msr is %b\n", msr_low);

	auto lapic_base = (volatile uint32_t *)
			MemoryMapper::new_address_map(lapic_addr, 0x3f4, lapic_flags, kernel_virt_allocator, allocators.general());

	fprintf(stdserial, "lapic spiv at %p\n", &lapic_base[lapic_registers::SPURIOUS_INTERRUPT_VECTOR]);
	lapic_base[lapic_registers::SPURIOUS_INTERRUPT_VECTOR] = 0x1ff;

	uint8_t bsp_core_id;
	__asm__ volatile("mov $1, %%eax; cpuid; shrl $24, %%ebx;" : "=b"(bsp_core_id) : : "eax", "ecx", "edx");
	fprintf(stdserial, "BSP id is %d\n", bsp_core_id);
	fprintf(stdserial, "ap running count is %d\n", *real_ap_running_count);

	for (auto core_id : processor_ids) {
		if (core_id == bsp_core_id) continue;

		fprintf(stdserial, "Booting core %d\n", core_id);

		// Send INIT IPI
		lapic_base[lapic_registers::ERROR_STATUS] = 0;
		lapic_base[lapic_registers::ICR_HIGH] = (lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
		lapic_base[lapic_registers::ICR_LOW]  = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x00C500;
		do {
			__asm__ volatile("pause" ::: "memory");
		} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12));   // Wait for delivered bit to clear

		// Deassert INIT IPI
		lapic_base[lapic_registers::ICR_HIGH] = (lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
		lapic_base[lapic_registers::ICR_LOW]  = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x008500;
		do {
			__asm__ volatile("pause" ::: "memory");
		} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12));   // Wait for delivered bit to clear

		get_local_data()->scheduler.sleep_ns(10'000'000);

		for (int i = 0; i < 2; i++) {
			// Send SIPI
			lapic_base[lapic_registers::ERROR_STATUS] = 0;
			lapic_base[lapic_registers::ICR_HIGH] =
					(lapic_base[lapic_registers::ICR_HIGH] & 0x00ffffff) | (core_id << 24);
			lapic_base[lapic_registers::ICR_LOW] = (lapic_base[lapic_registers::ICR_HIGH] & 0xfff00000) | 0x608;
			get_local_data()->scheduler.sleep_ns(200'000);
			do {
				__asm__ volatile("pause" ::: "memory");
			} while (lapic_base[lapic_registers::ICR_LOW] & (1 << 12));   // Wait for delivered bit to clear
		}
	}

	*real_ap_wait_flag = 1;
	get_local_data()->scheduler.sleep_ns(5'000'000);
	fprintf(stdserial, "ap running count is %d\n", *real_ap_running_count);

	// Switch to userspace, and stack switch, and call init
	/*auto userspace_stack_top = ktask->get_code_stack().start_page.end();
	// place init function and arguments at top of stack
	*(static_cast<uint64_t *>(userspace_stack_top) - 1) = initramfs_address.address();
	*(static_cast<uint64_t *>(userspace_stack_top) - 2) = boot_module->module_size();
	*(static_cast<uint64_t *>(userspace_stack_top) - 3) = reinterpret_cast<uint64_t>(uinit);
	// place userpace switch
	*(static_cast<uint64_t *>(userspace_stack_top) - 4) = reinterpret_cast<uint64_t>(threads::switch_to_user_mode);
	auto new_stack_ptr                                  = userspace_stack_top - 4 * 8;

	__asm__ volatile("xor %%rbp, %%rbp; mov %%rax, %%rsp; ret;" : : "a"(new_stack_ptr));*/
#endif
	throw "Unreachable code";
} catch (std::exception& e) {
	printf(TERMCOLOR_MAGENTA "Exception hit kmain: " TERMCOLOR_RESET "%s\n", e.what());
	throw;
}
