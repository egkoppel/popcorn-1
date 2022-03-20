#ifndef _HUGOS_TSS_H
#define _HUGOS_TSS_H

#include <stdint.h>

namespace tss {
	struct __attribute__((packed)) TSS {
		uint32_t _res0;
		uint64_t privilege_stack_table[3];
		uint64_t _res1;
		uint64_t interrupt_stack_table[7];
		uint64_t _res2;
		uint16_t _res3;
		uint16_t io_map_base;

		TSS();
		static void load(uint16_t gdt_index);
	};
}

#endif
