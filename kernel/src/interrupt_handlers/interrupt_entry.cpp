
#include "double_fault.hpp"
#include "page_fault.hpp"

#include <arch/interrupts.hpp>

extern "C" void exception_handler_entry(arch::interrupt_info_t *info) noexcept {
	switch (info->vector) {
		case 0x8: interrupt_handlers::double_fault(info); break;
		case 0xE: interrupt_handlers::page_fault(info); break;
		default: LOG(Log::INFO, "Unhandled irq at vector 0x%llx", info->vector); break;
	}
}
