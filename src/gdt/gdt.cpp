#include "gdt.hxx"
#include <assert.h>

using namespace gdt;

typedef struct __attribute__((packed)) {
	uint16_t size;
	uint64_t address;
} gdt_ptr;

entry::entry() {
	this->limit_low = 0;
	this->addr_low = 0;
	this->addr_mid = 0;
	this->access_byte = 0;
	this->limit_high  = 0;
	this->reserved = 0;
	this->long_mode = 0;
	this->db = 0;
	this->granularity = 0;
	this->addr_high = 0;
}

GDT::GDT() {
	this->next_free_entry = 1;
	uint64_t zero = 0;
	for (int i = 0; i < 8; ++i) this->entries[i] = entry();
}

void GDT::load() {
	gdt_ptr ptr;

	ptr.size = sizeof(GDT) - 1;
	ptr.address = reinterpret_cast<uint64_t>(this);

	__asm__ volatile("lgdt %0" : : "m"(ptr));
	__asm__ volatile("pushq $0x8; leaq .1$(%%rip), %%rax; pushq %%rax; lretq; .1$:" : : : "rax");
}

access_byte_user::operator uint8_t() {
	return *reinterpret_cast<uint8_t*>(this);
}

access_byte_system::operator uint8_t() {
	return *reinterpret_cast<uint8_t*>(this);
}

entry entry::new_code_segment(uint8_t dpl) {
	access_byte_user access_byte = {
		.access = 0,
		.writeable = 0,
		.conforming = 0,
		.executable = 1,
		.S = 1,
		.dpl = dpl,
		.present = 1
	};

	entry ret = {
		.limit_low = 0xFFFF,
		.addr_low = 0,
		.addr_mid = 0,
		.access_byte = access_byte,
		.limit_high = 0xF,
		.reserved = 0,
		.long_mode = 1,
		.db = 0,
		.granularity = 1,
		.addr_high = 0
	};

	return ret;
}

entry entry::new_data_segment(uint8_t dpl) {
	access_byte_user access_byte = {
		.access = 0,
		.writeable = 0,
		.conforming = 0,
		.executable = 0,
		.S = 1,
		.dpl = dpl,
		.present = 1
	};

	entry ret = {
		.limit_low = 0xFFFF,
		.addr_low = 0,
		.addr_mid = 0,
		.access_byte = access_byte,
		.limit_high = 0xF,
		.reserved = 0,
		.long_mode = 1,
		.db = 0,
		.granularity = 1,
		.addr_high = 0
	};

	return ret;
}

tss_entry::tss_entry(uint64_t addr, uint32_t size, uint8_t dpl) {
	access_byte_system access_byte = {
		.type = entry_type::TSS_AVAILABLE,
		.S = 0,
		.dpl = dpl,
		.present = 1
	};

	entry low = {
		.limit_low = size - 1,
		.addr_low = addr,
		.addr_mid = addr >> 16,
		.access_byte = access_byte,
		.limit_high = (size - 1) >> 16,
		.reserved = 0,
		.long_mode = 1,
		.db = 0,
		.granularity = 1,
		.addr_high = addr >> 24
	};

	uint64_t high = addr >> 32;

	this->low = low;
	this->high = *reinterpret_cast<entry*>(&high);
}

uint8_t GDT::add_entry(entry entry) {
	assert(this->next_free_entry < 8, "GDT is full");
	uint8_t index = this->next_free_entry;
	this->entries[index] = entry;
	this->next_free_entry++;
	return index;
}

uint8_t GDT::add_tss_entry(tss_entry entry) {
	assert(this->next_free_entry < 7, "GDT is full");
	uint8_t index = this->next_free_entry;
	this->entries[index] = entry.low;
	this->entries[index + 1] = entry.high;
	this->next_free_entry += 2;
	return index;
}
