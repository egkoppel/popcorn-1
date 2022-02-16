#include "idt.h"
#include <stdio.h>
#include <stdint.h>

IDT idt;

typedef struct __attribute__((packed)) {
	uint64_t error_code;
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
} exception_stack_frame_error;

typedef struct __attribute__((packed)) {
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
} exception_stack_frame;

#define N0_RETURN_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame_error *frame) { body } \
	__attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("cli; hlt"); \
	} \

#define N0_RETURN_NO_ERROR_CODE(name, body) \
		void name ## _inner(exception_stack_frame *frame) { body } \
	__attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("cli; hlt"); \
	}

#define RETURN_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame_error *frame) { body } \
	__attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
		__asm__ volatile("iretq"); \
	}

#define RETURN_NO_ERROR_CODE(name, body) \
	void name ## _inner(exception_stack_frame *frame) { body } \
	__attribute__((naked)) void name() { \
		__asm__ volatile("movq %rsp, %rdi"); \
		__asm__ volatile("call %P0" : : "i"(name ## _inner)); \
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

N0_RETURN_ERROR_CODE(double_fault_handler, {
	kfprintf(stdserial, "Double fault!\n");
	kfprintf(stdserial, "Error code: %d\n", frame->error_code);
	kfprintf(stdserial, "IP: %p\n", frame->ip);
	kfprintf(stdserial, "CS: 0x%04x\n", frame->cs);
	kfprintf(stdserial, "Flags: 0x%08x\n", frame->flags);
	kfprintf(stdserial, "SP: %p\n", frame->sp);
	kfprintf(stdserial, "SS: 0x%04x\n", frame->ss);
	while (1);
})

RETURN_ERROR_CODE(page_fault_handler, {
	kfprintf(stdserial, "Page fault!\n");
	kfprintf(stdserial, "Error code: %d\n", frame->error_code);
	kfprintf(stdserial, "IP: %p\n", frame->ip);
	kfprintf(stdserial, "CS: 0x%04x\n", frame->cs);
	kfprintf(stdserial, "Flags: 0x%08x\n", frame->flags);
	kfprintf(stdserial, "SP: %p\n", frame->sp);
	kfprintf(stdserial, "SS: 0x%04x\n", frame->ss);
	uint64_t cr2;
	__asm__ volatile("movq %%cr2, %0" : "=r"(cr2));
	kfprintf(stdserial, "Attempted access to: %p\n", cr2);
	while (1);
})

__attribute__((constructor)) void init_idt() {
	idt_init(&idt);
	idt_add_entry(&idt, 0x8, 0, double_fault_handler);
	idt_add_entry(&idt, 0xe, 0, page_fault_handler);
	idt_load(&idt);
}