#include "gdt.hpp"
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

entry::entry(uint64_t addr, uint32_t limit, uint8_t access_byte, uint8_t db, uint8_t granularity) {
	this->limit_low = limit;
	this->addr_low = addr;
	this->addr_mid = addr >> 16;
	this->access_byte = access_byte;
	this->limit_high = limit >> 16;
	this->reserved = 0;
	this->long_mode = 1;
	this->db = db;	
	this->granularity = granularity;
	this->addr_high = addr >> 24;
}

GDT::GDT() {
	this->next_free_entry = 1;
	for (int i = 0; i < 8; ++i) this->entries[i] = entry();
}

void GDT::load() {
	gdt_ptr ptr;

	ptr.size = sizeof(GDT) - 1;
	ptr.address = reinterpret_cast<uint64_t>(this);

	__asm__ volatile("lgdt %0" : : "m"(ptr));
	__asm__ volatile("pushq $0x8; leaq .1$(%%rip), %%rax; pushq %%rax; lretq; .1$:" : : : "rax");
}

access_byte_user_code::operator uint8_t() {
	return *reinterpret_cast<uint8_t*>(this);
}

access_byte_user_data::operator uint8_t() {
	return *reinterpret_cast<uint8_t*>(this);
}

access_byte_system::operator uint8_t() {
	return *reinterpret_cast<uint8_t*>(this);
}

entry entry::new_code_segment(uint8_t dpl) {
	access_byte_user_code access_byte = {
		.accessed = 0,
		.readable = 0,
		.conforming = 0,
		.executable = 1,
		.S = 1,
		.dpl = dpl,
		.present = 1
	};

	return entry(0, 0xFFFFF, access_byte, 0, 1);
}

entry entry::new_data_segment(uint8_t dpl) {
	access_byte_user_data access_byte = {
		.accessed = 0,
		.writeable = 1,
		.expand_down = 0,
		.executable = 0,
		.S = 1,
		.dpl = dpl,
		.present = 1
	};

	return entry(0, 0xFFFFF, access_byte, 0, 1);
}

tss_entry::tss_entry(uint64_t addr, uint32_t size, uint8_t dpl) {
	access_byte_system access_byte = {
		.type = entry_type::TSS_AVAILABLE,
		.S = 0,
		.dpl = dpl,
		.present = 1
	};

	entry low = entry(addr, size - 1, access_byte, 0, 1);

	uint64_t high = addr >> 32;

	this->low = low;
	this->high = *reinterpret_cast<entry*>(&high);
}

uint8_t GDT::add_entry(entry entry) {
	assert_msg(this->next_free_entry < 8, "GDT is full");
	uint8_t index = this->next_free_entry;
	this->entries[index] = entry;
	this->next_free_entry++;
	return index;
}

uint8_t GDT::add_tss_entry(tss_entry entry) {
	assert_msg(this->next_free_entry < 7, "GDT is full");
	uint8_t index = this->next_free_entry;
	this->entries[index] = entry.low;
	this->entries[index + 1] = entry.high;
	this->next_free_entry += 2;
	return index;
}
