#include "idt.hpp"
#include <stdio.h>
#include <stdint.h>
#include <panic.h>
#include "pic.hpp"

idt::IDT interrupt_descriptor_table = idt::IDT<48>();
pic::ATChainedPIC pics(0x20, 0x28);

struct __attribute__((packed)) exception_stack_frame_error {
	uint64_t error_code;
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
};

struct __attribute__((packed)) exception_stack_frame {
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
};

#define N0_RETURN_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame_error *frame) { body } \
	extern "C" __attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("cli; hlt"); \
	} \

#define N0_RETURN_NO_ERROR_CODE(name, body) \
		void name ## _inner(exception_stack_frame *frame) { body } \
	extern "C" __attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("cli; hlt"); \
	}

#define RETURN_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame_error *frame) { body } \
	extern "C" __attribute__((naked)) void name() { \
		__asm__ volatile("pushq %rax"); \
		__asm__ volatile("pushq %rcx"); \
		__asm__ volatile("pushq %rdx"); \
		__asm__ volatile("pushq %rsi"); \
		__asm__ volatile("pushq %rdi"); \
		__asm__ volatile("pushq %r8"); \
		__asm__ volatile("pushq %r9"); \
		__asm__ volatile("pushq %r10"); \
		__asm__ volatile("pushq %r11"); \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("addq $72, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("popq %r11"); \
		__asm__ volatile("popq %r10"); \
		__asm__ volatile("popq %r9"); \
		__asm__ volatile("popq %r8"); \
		__asm__ volatile("popq %rdi"); \
		__asm__ volatile("popq %rsi"); \
		__asm__ volatile("popq %rdx"); \
		__asm__ volatile("popq %rcx"); \
		__asm__ volatile("popq %rax"); \
		__asm__ volatile("iretq"); \
	}

#define RETURN_NO_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame *frame) { body } \
	extern "C" __attribute__((naked)) void name() { \
		__asm__ volatile("pushq %rax"); \
		__asm__ volatile("pushq %rcx"); \
		__asm__ volatile("pushq %rdx"); \
		__asm__ volatile("pushq %rsi"); \
		__asm__ volatile("pushq %rdi"); \
		__asm__ volatile("pushq %r8"); \
		__asm__ volatile("pushq %r9"); \
		__asm__ volatile("pushq %r10"); \
		__asm__ volatile("pushq %r11"); \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("addq $72, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("popq %r11"); \
		__asm__ volatile("popq %r10"); \
		__asm__ volatile("popq %r9"); \
		__asm__ volatile("popq %r8"); \
		__asm__ volatile("popq %rdi"); \
		__asm__ volatile("popq %rsi"); \
		__asm__ volatile("popq %rdx"); \
		__asm__ volatile("popq %rcx"); \
		__asm__ volatile("popq %rax"); \
		__asm__ volatile("iretq"); \
	}

/**
 * Exceptions types:
 * 0x00 - divide error (no error code)
 * 0x01 - debug exception (no error code)
 * 0x02 - non-maskable interrupt (no error code)
 * 0x03 - breakpoint (no error code)
 * 0x04 - overflow (no error code)
 * 0x05 - bound range exceeded (no error code)
 * 0x06 - invalid opcode (no error code)
 * 0x07 - device not available (no error code)
 * 0x08 - double fault (error code)
 * 0x09 - coprocessor segment overrun (no error code)
 * 0x0A - invalid TSS (error code)
 * 0x0B - segment not present (error code)
 * 0x0C - stack-segment fault (error code)
 * 0x0D - general protection fault (error code)
 * 0x0E - page fault (error code)
 * 0x0F - reserved (no error code)
 */

extern uint8_t level4_page_table;

N0_RETURN_ERROR_CODE(double_fault_handler, {
	fprintf(stdserial, "Double fault!\n");
	fprintf(stdserial, "Error code: %d\n", frame->error_code);
	fprintf(stdserial, "IP: %lp\n", frame->ip);
	fprintf(stdserial, "CS: 0x%04x\n", frame->cs);
	fprintf(stdserial, "Flags: 0x%08x\n", frame->flags);
	fprintf(stdserial, "SP: %lp\n", frame->sp);
	fprintf(stdserial, "SS: 0x%04x\n", frame->ss);

	uint64_t cr2;
	__asm__ volatile("movq %%cr2, %0" : "=r"(cr2));

	if (cr2 >= reinterpret_cast<uint64_t>(&level4_page_table) && cr2 < reinterpret_cast<uint64_t>(&level4_page_table) + 0x1000) {
		fprintf(stdserial, "CR2: %lp - possible stack overflow\n", cr2);
		panic("Potential stack overflow");
	}

	while (1);
})

RETURN_ERROR_CODE(page_fault_handler, {
	fprintf(stdserial, "Page fault!\n");
	fprintf(stdserial, "Error code: %d\n", frame->error_code);
	fprintf(stdserial, "IP: %lp\n", frame->ip);
	fprintf(stdserial, "CS: 0x%04x\n", frame->cs);
	fprintf(stdserial, "Flags: 0x%08x\n", frame->flags);
	fprintf(stdserial, "SP: %lp\n", frame->sp);
	fprintf(stdserial, "SS: 0x%04x\n", frame->ss);
	uint64_t cr2;
	__asm__ volatile("movq %%cr2, %0" : "=r"(cr2));
	fprintf(stdserial, "Attempted access to: %lp\n", cr2);
	while (1);
})

namespace PIC_IRQ { enum PIC_IRQ {
	PIC1 = 0x20,
	TIMER = PIC1,
	KEYBOARD = PIC1 + 1,
	SERIAL2 = PIC1 + 3,
	SERIAL1 = PIC1 + 4,
	PARALLEL2_3 = PIC1 + 5,
	FD = PIC1 + 6,
	PARALLEL1 = PIC1 + 7,

	PIC2 = 0x28,
	RTC = PIC2,
	ACPI = PIC2 + 1,
	MOUSE = PIC2 + 4,
	X87 = PIC2 + 5,
	ATA_P = PIC2 + 6,
	ATA_S = PIC2 + 7
};}

RETURN_NO_ERROR_CODE(timer_interrupt_handler, {
	printf("-");
	pics.acknowledge_irq(PIC_IRQ::TIMER);
})

void init_idt() {
	interrupt_descriptor_table.add_entry(0x8, 0, double_fault_handler, 1);
	interrupt_descriptor_table.add_entry(0xe, 0, page_fault_handler);
	interrupt_descriptor_table.add_entry(PIC_IRQ::TIMER, 0, timer_interrupt_handler);
	interrupt_descriptor_table.load();
}
