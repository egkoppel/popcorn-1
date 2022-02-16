bits 32

KERNEL_VIRTUAL_BASE: equ 0xFFFFFF8000000000

section .rodata

global lm_enabled
global hello_world_str
hello_world_str db "Hello World!", 0
color db 0x0f

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

gdt64:
	.null:
		dq 0 ; null entry
	.code_kernel:
		dd 0xFFFF ; limit low
		db 0 ; base low
		db .PRESENT | .S_BIT | .EXEC | .RW ; present, S bit, code, read/write, ring 0
		db .GRANULARITY | .LONG_MODE | 0xF ; granular, long mode, limit high
		db 0 ; base high
	.data_kernel:
		dd 0xFFFF ; limit low
		db 0 ; base low
		db .PRESENT | .S_BIT | .RW ; present, S bit, data, read/write, ring 0
		db .GRANULARITY | .S_BIT | 0xF ; granular, long mode, limit high
		db 0 ; base high
	.code_user:
		dd 0xFFFF ; limit low
		db 0 ; base low
		db .PRESENT | .S_BIT | .EXEC | .RW| .RING_3 ; present, S bit, code, read/write, ring 3
		db .GRANULARITY | .LONG_MODE | 0xF ; granular, long mode, limit high
		db 0 ; base high
	.data_user:
		dd 0xFFFF ; limit low
		db 0 ; base low
		db .PRESENT | .S_BIT | .RW | .RING_3 ; present, S bit, data, read/write, ring 3
		db .GRANULARITY | .S_BIT | 0xF ; granular, long mode, limit high
		db 0 ; base high
	.pointer:
		dw $ - gdt64 - 1 - KERNEL_VIRTUAL_BASE
		dq gdt64 - KERNEL_VIRTUAL_BASE

	.PRESENT		equ 1 << 7
	.RING_3			equ 3 << 5
	.S_BIT			equ 1 << 4
	.EXEC			equ 1 << 3
	.DC				equ 1 << 2
	.RW				equ 1 << 1
	.ACCESSED		equ 1 << 0

	.GRANULARITY	equ 1 << 7
	.SZ_32			equ 1 << 6
	.LONG_MODE		equ 1 << 5

section .text
global _start
extern _32_puts
extern _32_utoa
extern _32_print_cpu_info
extern long_mode_start

extern read_multiboot_info

extern framebuffer_addr
extern framebuffer_pitch
extern framebuffer_width
extern framebuffer_height
extern framebuffer_bpp
extern framebuffer_type

_start:
	mov esp, _stack_top - KERNEL_VIRTUAL_BASE
	mov ebp, esp
	push eax ; save multiboot magic
	push ebx ; save multiboot info
	
	push ebx ; pass multiboot info
	call read_multiboot_info
	add esp, 4 ; clear args

	; CHECK CPUID SUPPORTED
	;push cpuid_check ; print checking for cpuid
	mov eax, cpuid_check - KERNEL_VIRTUAL_BASE
	push eax
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
		mov eax, cpuid_not_found - KERNEL_VIRTUAL_BASE
		push eax
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.1: 
		mov eax, cpuid_found - KERNEL_VIRTUAL_BASE
		push eax
		call _32_puts
		add esp, 4 ; clear args
	

	; CHECK MAX CPUID EXTENDED FEATURESET SUPPORTED
	mov eax, cpuid_max_check - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args

	mov eax, 0x80000000
	cpuid
	push eax ; save max extended feature set

	cmp eax, 0x80000001
	jnb .2
		mov eax, cpuid_max_not_found - KERNEL_VIRTUAL_BASE
		push eax
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.2: 
		mov eax, cpuid_max_found - KERNEL_VIRTUAL_BASE
		push eax
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

		mov eax, nl - KERNEL_VIRTUAL_BASE
		push eax ; print newline
		call _32_puts
		add esp, 4 ; clear args
	

	; CHECK LONG MODE SUPPORTED
	mov eax, long_mode_check - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args

	mov eax, 0x80000001
	cpuid
	test edx, 1<<29 ; test LM bit
	jnz .3
		mov eax, long_mode_not_found - KERNEL_VIRTUAL_BASE
		push eax
		call _32_puts
		add esp, 4 ; clear args
		jmp error
	.3:
		mov eax, long_mode_found - KERNEL_VIRTUAL_BASE
		push eax
		call _32_puts
		add esp, 4 ; clear args

	; INIT PAGING
	mov eax, init_paging - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args

	mov eax, level3_page_table - KERNEL_VIRTUAL_BASE
	or eax, 0b11 ; set present and write bits
	mov [level4_page_table + 8*0 - KERNEL_VIRTUAL_BASE], eax ; map p4[0] to p3_0
	mov [level4_page_table + 8*511 - KERNEL_VIRTUAL_BASE], eax ; map p4[511] to p3_0

	mov eax, level2_page_table_0 - KERNEL_VIRTUAL_BASE
	or eax, 0b11 ; set present and write bits
	mov [level3_page_table + 8*0 - KERNEL_VIRTUAL_BASE], eax ; map p3[0] to p2_0

	mov eax, level2_page_table_1 - KERNEL_VIRTUAL_BASE
	or eax, 0b11 ; set present and write bits
	mov [level3_page_table + 8*1 - KERNEL_VIRTUAL_BASE], eax ; map p3[1] to p2_1

	; map each p2_0 entry to a huge page (2MiB)
	mov ecx, 0
	.map_level2_0_table_loop:
		mov eax, 0x200000 ; size of each entry (2MiB)
		mul ecx ; real start address of entry (counter * 2MiB)
		or eax, 0b10000011 ; present, write, and huge bits
		mov [level2_page_table_0 + ecx * 8 - KERNEL_VIRTUAL_BASE], eax ; map p2[counter] to eax

		inc ecx
		cmp ecx, 512 ; 512 entries in p2
	jne .map_level2_0_table_loop

	; map each p2_1 entry to a huge page (2MiB)
	mov ecx, 0
	.map_level2_1_table_loop:
		mov eax, 0x200000 ; size of each entry (2MiB)
		mul ecx ; real start address of entry (counter * 2MiB)
		add eax, 0xfd000000 ; start mapping from 0xfd000000
		or eax, 0b10000011 ; present, write, and huge bits
		mov [level2_page_table_1 + ecx * 8 - KERNEL_VIRTUAL_BASE], eax ; map p2[counter] to eax

		inc ecx
		cmp ecx, 16 ; only map 4K 32bpp framebuffer
	jne .map_level2_1_table_loop

	mov eax, init_paging_done - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args

	; enable paging
	mov eax, level4_page_table - KERNEL_VIRTUAL_BASE
	mov cr3, eax ; set p4 as l4 page table

	; enable PAE
	mov eax, cr4
	or eax, 1<<5 ; set PAE bit
	mov cr4, eax

	mov eax, enable_paging_done - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args

	; ENTER LONG MODE
	mov eax, lm_enabling - KERNEL_VIRTUAL_BASE
	push eax
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
	lgdt [gdt64.pointer - KERNEL_VIRTUAL_BASE]
	mov eax, lm_gdt_load - KERNEL_VIRTUAL_BASE
	push eax
	call _32_puts
	add esp, 4 ; clear args
	jmp 0x8:long_mode_start - KERNEL_VIRTUAL_BASE
error:
loop: hlt
	jmp loop

section .bss
align 4096
global _stack_top
level4_page_table:
	resb 0x1000
level3_page_table:
	resb 0x1000
level2_page_table_0:
	resb 0x1000
level2_page_table_1:
	resb 0x1000
_stack_bottom:
	resb 0x1000*4
_stack_top: