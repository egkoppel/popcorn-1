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
#include <memory/physical_allocators/null_allocator.hpp>
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
#include <serial.h>
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

void parse_cli_args(const multiboot::Data& multiboot) {
	auto cli = multiboot.find_tag<multiboot::tags::Cli>(multiboot::TagType::CLI);

	const char *cli_data = "";
	if (cli) { cli_data = cli.value()->args(); }

	LOG(Log::INFO, "Received CLI args: %s", cli_data);

	const char log_level_str[] = "log_level=";
	const char *log_level_ptr  = strstr(cli_data, log_level_str);
	if (log_level_ptr) {
		auto log_level = strtol(log_level_ptr + sizeof(log_level_str) - 1, nullptr, 10);
		Log::set_log_level(static_cast<Log::level_t>(log_level));
		LOG(Log::DEBUG, "Set log level to %d", log_level);
		LOG(Log::TRACE, "%s", log_level_ptr + sizeof(log_level_str) - 1);
	}
}

void parse_bootloader(const multiboot::Data& multiboot) {
	auto bootloader = multiboot.find_tag<multiboot::tags::Bootloader>(multiboot::TagType::BOOTLOADER_NAME);
	if (bootloader) { LOG(Log::INFO, "Booted by %s", bootloader->name()); }
}

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
 *  - 4M of memory mapped starting at `memory::constants::mem_map_start` - MUST NOT OVERLAP WITH ANY OTHER KERNEL CONSTRUCTS
 */
extern "C" void kmain(u32 multiboot_magic, paddr32_t multiboot_addr) {
	Log::set_log_level(Log::INFO);
	Log::set_screen_log_level(Log::INFO);

	try {
		serial1 = SerialPort{0x3f8};
	} catch (SerialPort::SerialPortError&) { LOG(Log::WARNING, "Serial port failed test"); }

	if (multiboot_magic == 0x36d76289) {
		LOG(Log::INFO, "Multiboot magic: 0x36d76289 (correct)");
	} else {
		LOG(Log::WARNING, "Multiboot magic: 0x%x (incorrect)", multiboot_magic);
	}

	// FIXME: why the flip does this not work
	paddr_t _a        = multiboot_addr;
	decltype(auto) mb = *static_cast<const multiboot::Data *>(_a.virtualise());
	LOG(Log::DEBUG, "Multiboot info struct loaded at %lp", _a);

	parse_cli_args(mb);
	parse_bootloader(mb);

	auto fb = mb.find_tag<multiboot::tags::Framebuffer>(multiboot::TagType::FRAMEBUFFER).value();

	auto sections = mb.find_tag<multiboot::tags::ElfSections>(multiboot::TagType::ELF_SECTIONS).value();
	/*TODO
	 * auto boot_module = mb.find_tag<multiboot::tags::BootModule>(multiboot::TagType::BOOT_MODULE).value();
     * if (strcmp(boot_module->name(), "initramfs") != 0) panic("No initramfs found");*/

	arch::arch_specific_early_init();
	arch::load_interrupt_handler(arch::InterruptVectors::PAGE_FAULT, false, 0, interrupt_handlers::page_fault);
	arch::load_interrupt_handler(arch::InterruptVectors::DOUBLE_FAULT, false, 1, interrupt_handlers::double_fault);
	arch::load_interrupt_handler(arch::InterruptVectors::CORE_TIMER, false, 0, [](arch::interrupt_info_t *) noexcept {
		threads::local_scheduler->irq_fired();
	});

	LOG(Log::DEBUG, "Finished architecture init");

	LOG(Log::DEBUG, "Initialising memory");
	auto kernel_max = 0_pa;
	auto kernel_min = 0xffff'ffff'ffff'ffff_pa;
	usize tls_size  = 0;
	for (auto& i : *sections) {
		if ((i.type() != decltype(i.type())::SHT_NULL)
		    && (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_ALLOC) != 0) {
			stdserial << i;
			stdout << i;

			auto name            = i.name(*sections);
			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset      = 0;
			if (!is_ap_bootstrap) offset = memory::constants::kexe_start;

			if (i.start() - offset < kernel_min) kernel_min = i.start() - offset;
			if (i.end() - offset > kernel_max) kernel_max = i.end() - offset;

			if (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_TLS) {
				tls_size = i.end().address - i.start().address;
			}
		}
	}
	LOG(Log::DEBUG, "Kernel executable: %lp -> %lp", kernel_min, kernel_max);
	LOG(Log::DEBUG, "TLS size: %llu", tls_size);

	uint64_t available_ram = 0;
	uint64_t total_ram     = 0;
	auto mmap              = mb.find_tag<multiboot::tags::MemoryMap>(multiboot::TagType::MEMORY_MAP).value();
	for (auto& entry : *mmap) {
		if (entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
			available_ram += entry.get_size();

			if (entry.get_end_address().address > total_ram) total_ram = entry.get_end_address().address;

			LOG(Log::DEBUG,
			    "\t%lp - %lp (%s)",
			    entry.get_start_address(),
			    entry.get_end_address(),
			    entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE ? "AVAILABLE" : "RESERVED");
		}
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
	allocators.general_frame_allocator_   = &kernel_monotonic_frame_allocator;

	// ******************************* BEGIN PAGE TABLE REMAPPING *******************************
	auto& new_p4_table = paging::init_kas(kernel_monotonic_frame_allocator);

	// Map the kernel with section flags
	for (auto& i : *sections) {
		if ((i.type() != multiboot::tags::ElfSections::Entry::Type::SHT_NULL)
		    && (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_ALLOC) != 0
		    && (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_TLS) == 0) {
			auto name         = i.name(*sections);
			auto is_userspace = strncmp(name, ".userspace", 10) == 0;

			auto is_ap_bootstrap = strncmp(name, ".ap_bootstrap", 13) == 0;
			uint64_t offset      = 0;
			if (!is_ap_bootstrap) offset = memory::constants::kexe_start;

			for (auto phys_frame = aligned<paddr_t>::aligned_down(i.start() - offset);
			     phys_frame < aligned<paddr_t>::aligned_up(i.end() - offset);
			     phys_frame++) {
				using enum memory::paging::PageTableFlags;
				auto flags = static_cast<paging::PageTableFlags>(0);
				if (i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_WRITE) flags = flags | WRITEABLE;
				if (is_userspace || USER_ACCESS_FROM_KERNEL) flags = flags | USER;
				flags = flags | GLOBAL;
				if (!(i.flags() & +multiboot::tags::ElfSections::Entry::Flags::SHF_EXECINSTR))
					flags = flags | NO_EXECUTE;

				LOG(Log::TRACE,
				    "Kexe - Mapping %p -> %p",
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

	allocators.general_virtual_allocator_ = &kernel_virt_allocator;

	memory::physical_allocators::NullAllocator null_allocator{};

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

	LOG(Log::DEBUG, "Map all the memory");
	auto all_mem_flags = paging::PageTableFlags::WRITEABLE | memory::paging::PageTableFlags::GLOBAL
	                     | memory::paging::PageTableFlags::NO_EXECUTE;
	if (USER_ACCESS_FROM_KERNEL) all_mem_flags = all_mem_flags | memory::paging::PageTableFlags::USER;

	/*for (auto f = 0_palign; f.address.address < total_ram; f++) {
		new_p4_table.map_page_to(vaddr_t{.address = f.address.address + memory::constants::page_offset_start},
		                         f.frame(),
		                         all_mem_flags);
	}*/
	for (auto& entry : *mmap) {
		if (entry.get_type() == multiboot::tags::MemoryMap::Type::AVAILABLE) {
			auto start = aligned<paddr_t>::aligned_down(entry.get_start_address());
			auto end   = aligned<paddr_t>::aligned_up(entry.get_end_address());

			LOG(Log::DEBUG, "(%lp -> %lp)", start, end);

			auto count_total = (end.address.address - start.address.address) / constants::frame_size;
			auto one_percent = count_total / 50;
			usize i          = 0;

			Log::off();
			for (auto f = start; f < end; f++) {
				new_p4_table.map_page_to(vaddr_t{.address = f.address.address + memory::constants::page_offset_start},
				                         f.frame(),
				                         all_mem_flags);

				if (++i % one_percent == 0) {
					Log::on();
					LOG(Log::DEBUG, "%d%%", i / one_percent);
					Log::off();
				}
			}
			Log::on();
		}
	}

	LOG(Log::DEBUG, "Map early mem_map region");
	auto mem_map_flags = paging::PageTableFlags::WRITEABLE | memory::paging::PageTableFlags::GLOBAL
	                     | memory::paging::PageTableFlags::NO_EXECUTE;
	if (USER_ACCESS_FROM_KERNEL) mem_map_flags = mem_map_flags | memory::paging::PageTableFlags::USER;
	aligned<vaddr_t> page = vaddr_t{.address = constants::mem_map_start};
	for (aligned<paddr_t> frame = real_initial_mem_map_start; frame < real_initial_mem_map_start + 0x400000;
	     page++, frame++) {
		new_p4_table.map_page_to(page, frame.frame(), mem_map_flags);
	}

	// ******************************* END PAGE TABLE REMAPPING *******************************

	/* TODO: Check if needs address offset like old code
	 * PhysicalAddress old_p4_table_frame;
	 * __asm__ volatile("mov %%cr3, %0" : "=r"(old_p4_table_frame));
	 * Page old_p4_table_page = Page::from_address(static_cast<VirtualAddress>(old_p4_table_frame + 0xFFFF800000000000));
	 * __asm__ volatile("mov %0, %%cr3" : : "r"(new_p4_table));
	 */
	fprintf(stdserial, "************\n");

	auto old_p4_table = get_current_page_table_addr();
	vaddr_t old_p4_table_page{.address = old_p4_table.address + constants::kexe_start};
	LOG(Log::INFO, "Creating stack guard page at %lp", old_p4_table_page);
	auto _ = new_p4_table.unmap_page(old_p4_table_page);
	LOG(Log::INFO, "Created? %i", _);

	Log::set_screen_log_level(Log::OFF);
	new_p4_table.make_active();

	auto framebuffer_flags = paging::PageTableFlags::WRITEABLE | memory::paging::PageTableFlags::NO_EXECUTE
	                         | memory::paging::PageTableFlags::IMPL_CACHE_DISABLE
	                         | memory::paging::PageTableFlags::IMPL_CACHE_WRITETHROUGH
	                         | memory::paging::PageTableFlags::GLOBAL;
	MemoryMap<u8> framebuffer_mapping{fb->begin(), fb->size(), framebuffer_flags, null_allocator, paging::kas};

	FRAMEBUFFER = reinterpret_cast<char *>(framebuffer_mapping.operator->());
	Log::set_screen_log_level(Log::INFO);
	LOG(Log::WARNING, "hello???");

	auto rsdp_tag = mb.find_tag<multiboot::tags::Rsdp>(multiboot::TagType::RSDT_V1)
	                        .or_else([&] { return mb.find_tag<multiboot::tags::Rsdp>(multiboot::TagType::RSDT_V2); })
	                        .value();

	if (strncmp(reinterpret_cast<const char *>(&rsdp_tag->signature), "RSD PTR ", 8) != 0) {
		panic("RSDP signature wrong");
	}

	char oem_str_buf[7] = {0};
	memcpy(oem_str_buf, &rsdp_tag->oem_id, 6);
	LOG(Log::INFO, "OEM is %s\n", oem_str_buf);

	LOG(Log::DEBUG, "Initialise memory bitmap");
	uint64_t bitmap_needed_bytes = IDIV_ROUND_UP(total_ram / 0x1000, 8);
	auto main_frame_allocator    = memory::physical_allocators::BitmapAllocator<general_allocator_t>::from(
            0x100000_pa,
            bitmap_needed_bytes,
            std::move(kernel_monotonic_frame_allocator),
            general_allocator_t{});

	init_sbrk();

	allocators.general_frame_allocator_ = new physical_allocators::BitmapAllocator(std::move(main_frame_allocator));

	LOG(Log::DEBUG, "Creating extra stacks");

	KStack<> double_fault_stack{constants::frame_size /* * 2*/};   // TODO: Fix once bitmap allocator can allocate more
	arch::load_backup_stack(1, std::move(double_fault_stack));

	LOG(Log::DEBUG, "Loaded double fault stack");

	/*uint8_t index = global_descriptor_table.add_tss_entry(
			gdt::tss_entry(reinterpret_cast<uint64_t>(&task_state_segment), sizeof(tss::TSS), 0));
	printf("TSS index at %u\n", index);
	tss::TSS::load(index);*/
	//printf("[ " TERMCOLOR_GREEN "OK" TERMCOLOR_RESET " ] Loaded TSS\n");

	create_core_local_data(tls_size);


	while (true) __asm__ volatile("nop");
#if 0

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
	__builtin_unreachable();
}
