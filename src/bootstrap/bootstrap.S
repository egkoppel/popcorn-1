bits 32
section .rodata

global lm_enabled
global hello_world_str
hello_world_str db "Hello World!", 0
color db 0x0f
len dd (color - hello_world_str)

%define col_reset 128
%define col_red 129
%define col_green 130

nl db 0xa, 0

cpuid_check db "[    ] Checking for CPUID support", 0xa, 0
cpuid_found db "[", col_green, " OK ", col_reset, "] CPUID supported", 0xa, 0
cpuid_not_found db "[", col_red, "FAIL", col_reset, "] CPUID support not found", 0xa, 0

cpuid_max_check db "[    ] Checking for extended CPUID support", 0xa, 0
cpuid_max_found db "[", col_green, " OK ", col_reset, "] Extended CPUID supported. Max feature: 0x", 0
cpuid_max_not_found db "[", col_red, "FAIL", col_reset, "] Extended CPUID support not found", 0xa, 0

long_mode_check db "[    ] Checking for long mode support", 0xa, 0
long_mode_found db "[", col_green, " OK ", col_reset, "] Long mode supported", 0xa, 0
long_mode_not_found db "[", col_red, "FAIL", col_reset, "] Long mode not supported", 0xa, 0

init_paging db "[    ] Initializing paging", 0xa, 0
init_paging_done db "[", col_green, " OK ", col_reset, "] Paging initialized", 0xa, 0
enable_paging_done db "[", col_green, " OK ", col_reset, "] Paging enabled", 0xa, 0

lm_enabling db "[    ] Entering long mode", 0xa, 0
lm_gdt_load db "[", col_green, " OK ", col_reset, "] Loaded GDT", 0xa, 0

PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0

GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

gdt64:
	.null: equ $ - gdt64
		dq 0 ; null entry
	.code_kernel: equ $ - gdt64
		dd 0xFFFF
		db 0
		db PRESENT | NOT_SYS | EXEC | RW
		db GRAN_4K | LONG_MODE | 0xF
		db 0
	.data_kernel: equ $ - gdt64
		dd 0xFFFF
		db 0
		db PRESENT | NOT_SYS | RW
		db GRAN_4K | SZ_32 | 0xF
		db 0
	.tss: equ $ - gdt64
		dd 0x00000068
		dd 0x00CF8900
	.pointer:
		dw $ - gdt64 - 1
		dq gdt64

section .text
global _start
extern _32_puts
extern _32_utoa
extern _32_print_cpu_info
extern long_mode_start

_start:
	mov esp, _stack_top
	mov ebp, esp
	push eax ; save multiboot magic
	push ebx ; save multiboot info

	; CHECK CPUID SUPPORTED
	push cpuid_check ; print checking for cpuid
	call _32_puts
	add esp, 4 ; clear args
	
	pushfd
	pop eax ; move flags into eax
	mov ebx, eax ; save old flags
	xor eax, 1<<21 ; flip bit 21
	push eax
	popfd ; write back flags

	pushfd
	pop eax ; move modified flags into eax
	push ebx
	popfd ; restore old flags

	xor eax, ebx
	jnz .1
		push cpuid_not_found
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.1: 
		push cpuid_found
		call _32_puts
		add esp, 4 ; clear args
	

	; CHECK MAX CPUID EXTENDED FEATURESET SUPPORTED
	push cpuid_max_check
	call _32_puts
	add esp, 4 ; clear args

	mov eax, 0x80000000
	cpuid
	push eax ; save max extended feature set

	cmp eax, 0x80000001
	jnb .2
		push cpuid_max_not_found
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.2: 
		push cpuid_max_found
		call _32_puts
		add esp, 4 ; clear args

		pop eax ; get max featureset
		push 16 ; print in base 16
		push eax ; push max featureset
		call _32_utoa
		add esp, 8 ; clear args

		push eax ; print result of itoa
		call _32_puts
		add esp, 4 ; clear args

		push nl ; print newline
		call _32_puts
		add esp, 4 ; clear args
	

	; CHECK LONG MODE SUPPORTED
	push long_mode_check
	call _32_puts
	add esp, 4 ; clear args

	mov eax, 0x80000001
	cpuid
	test edx, 1<<29 ; test LM bit
	jnz .3
		push long_mode_not_found
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.3:
		push long_mode_found
		call _32_puts
		add esp, 4 ; clear args


	;call print_cpu_info ; print cpu info


	; INIT PAGING
	push init_paging
	call _32_puts
	add esp, 4 ; clear args

	mov eax, level3_page_table
	or eax, 0b11 ; set present and write bits
	mov [level4_page_table], eax ; map p4[0] to p3

	mov eax, level2_page_table
	or eax, 0b11 ; set present and write bits
	mov [level3_page_table], eax ; map p3[0] to p2

	; map each p2 entry to a huge page (2MiB)
	mov ecx, 0
	.map_level2_table_loop:
		mov eax, 0x200000 ; size of each entry (2MiB)
		mul ecx ; real start address of entry (counter * 2MiB)
		or eax, 0b10000011 ; present, write, and huge bits
		mov [level2_page_table + ecx * 8], eax ; map p2[counter] to eax

		inc ecx
		cmp ecx, 512 ; 512 entries in p2
	jne .map_level2_table_loop

	push init_paging_done
	call _32_puts
	add esp, 4 ; clear args

	; enable paging
	mov eax, level4_page_table
	mov cr3, eax ; set p4 as l4 page table

	; enable PAE
	mov eax, cr4
	or eax, 1<<5 ; set PAE bit
	mov cr4, eax

	push enable_paging_done
	call _32_puts
	add esp, 4 ; clear args


	; ENTER LONG MODE
	push lm_enabling
	call _32_puts
	add esp, 4 ; clear args

	mov ecx, 0xC0000080 ; EFER rregister
	rdmsr
	or eax, 1<<8 ; set long mode bit
	wrmsr

	mov eax, cr0 ; enable paging
	or eax, 1<<31 ; set paging bit
	mov cr0, eax

	; load gdt
	lgdt [gdt64.pointer]
	push lm_gdt_load
	call _32_puts
	add esp, 4 ; clear args
	jmp gdt64.code_kernel:long_mode_start
error:
loop: hlt
	jmp loop

section .bss
align 4096
level4_page_table:
	resb 0x1000
level3_page_table:
	resb 0x1000
level2_page_table:
	resb 0x1000
_stack_bottom:
	resb 0x1000*4
_stack_top: