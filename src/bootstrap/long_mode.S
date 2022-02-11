bits 64

global long_mode_start

%define col_reset 0x1b, "[0m"
%define col_red 0x1b, "[31m"
%define col_green 0x1b, "[32m"

section .rodata
sse_enabling db "[    ] Enabling SSE support", 0xa, 0
sse_enabled db "[", col_green, " OK ", col_reset, "] SSE enabled", 0xa, 0

section .text
extern __cxa_init
extern kputs
long_mode_start:
	; ENABLE SSE
	lea rdi, [rel sse_enabling]
	call kputs

	mov rax, cr0
	and ax, 0xFFFB ; clear CR0.EM
	and ax, 0x2 ; enable coprocessor monitoring
	mov cr0, rax

	mov rax, cr4
	or ax, 1<<9 ; enable SSE instructions
	or ax, 1<<10 ; enable SSE exceptions
	mov cr4, rax

	lea rdi, [rel sse_enabled]
	call kputs

	mov esi, [rbp-8] ; pop multiboot info
	mov edi, [rbp-4] ; pop multiboot magic
	mov rsp, rbp ; reset stack pointer to stack top
	call __cxa_init
	hlt