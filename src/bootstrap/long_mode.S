bits 64

global long_mode_start

section .rodata

section .text
extern __cxa_init
long_mode_start:
	mov esi, [rbp-8] ; pop multiboot info
	mov edi, [rbp-4] ; pop multiboot magic
	mov rsp, rbp ; reset stack pointer to stack top
	call __cxa_init
	hlt