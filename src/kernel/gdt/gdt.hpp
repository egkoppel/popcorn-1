#ifndef _HUGOS_GDT_H
#define _HUGOS_GDT_H

#include <stdint.h>

namespace gdt {
	enum class entry_type: uint8_t {
		LDT = 2,
		TSS_AVAILABLE = 9,
		TSS_BUSY = 0xb,
	};

	struct __attribute__((packed)) access_byte_system {
		entry_type type: 4;
		uint8_t S: 1;
		uint8_t dpl: 2;
		uint8_t present: 1;

		operator uint8_t();
	};

	struct __attribute__((packed)) access_byte_user_code {
		uint8_t accessed: 1;
		uint8_t readable: 1;
		uint8_t conforming: 1;
		uint8_t executable: 1;
		uint8_t S: 1;
		uint8_t dpl: 2;
		uint8_t present: 1;

		operator uint8_t();
	};

	struct __attribute__((packed)) access_byte_user_data {
		uint8_t accessed: 1;
		uint8_t writeable: 1;
		uint8_t expand_down: 1;
		uint8_t executable: 1;
		uint8_t S: 1;
		uint8_t dpl: 2;
		uint8_t present: 1;

		operator uint8_t();
	};

	struct __attribute__((packed)) entry {
		uint16_t limit_low;
		uint16_t addr_low;
		uint8_t addr_mid;
		uint8_t access_byte;
		uint8_t limit_high : 4;
		uint8_t reserved: 1;
		uint8_t long_mode: 1;
		uint8_t db: 1;
		uint8_t granularity: 1;
		uint8_t addr_high;

		entry();
		entry(uint64_t addr, uint32_t limit, uint8_t access_byte, uint8_t db, uint8_t granularity);
		static entry new_code_segment(uint8_t dpl);
		static entry new_data_segment(uint8_t dpl);
	};

	struct __attribute__((packed)) tss_entry {
		entry low, high;

		tss_entry(uint64_t addr, uint32_t size, uint8_t dpl);
	};

	struct __attribute__((packed, aligned(8))) GDT {
		entry entries[8];
		int next_free_entry;

		GDT();
		uint8_t add_entry(entry entry);
		uint8_t add_tss_entry(tss_entry entry);
		void load();
	};
}

#endif
