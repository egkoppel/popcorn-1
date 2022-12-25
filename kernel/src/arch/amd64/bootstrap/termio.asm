bits 32

KERNEL_VIRTUAL_BASE: equ 0xFFFFFFFF80000000

section .bss
global x
global y
global termsize_x
global termsize_y
x: resb 2
y: resb 2
termsize_x: resb 2 ; assume nobody has a display with more than 16383 * psf_width along either axis
termsize_y: resb 2
itoa_buffer: resb 32

section .rodata
itoa_str db "0123456789abcdefghijklmnopqrstuvwxyz"

section .data
global col
col dd 0xffffff

section .text
global _32_utoa
_32_utoa:
	; prologue
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi

	; [ebp+8] -> val
	; [ebp+12] -> base
	mov eax, dword [ebp+8]
	mov edi, 30 ; int i = 30

	.loop_start:
		; val -> eax
		; base -> [ebp+12]
		mov edx, 0 ; => dividend is 0:val
		div dword [ebp+12] ; val / base
		; val % base -> edx
		; val / base -> eax

		movzx ecx, byte [itoa_str + edx - KERNEL_VIRTUAL_BASE] ; cl = itoa_str[val % base]
		mov byte [itoa_buffer + edi - KERNEL_VIRTUAL_BASE], cl ; buf[i] = cl = itoa_str[val % base]

		dec edi ; --i
		; val /= base already done since quotient in eax

		cmp eax, 0 ; if (val == 0) break;
		je .loop_end

		cmp edi, 0 ; if (i < 0) break;
		jl .loop_end
	jmp .loop_start
	.loop_end:

	lea eax, [itoa_buffer + edi + 1 - KERNEL_VIRTUAL_BASE]; return &buf[i+1]

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilogue
	mov esp, ebp
	pop ebp
	ret

extern framebuffer_addr
extern framebuffer_width
extern framebuffer_height
extern framebuffer_bpp
extern psf_height
global _32_shift_up
_32_shift_up:
	; prologue
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi
	
	mov eax, [framebuffer_bpp - KERNEL_VIRTUAL_BASE]
	shr eax, 3
	imul dword [framebuffer_width - KERNEL_VIRTUAL_BASE]
	imul dword [psf_height - KERNEL_VIRTUAL_BASE]
	mov esi, eax ; esi = stride for one row of text
	imul dword [framebuffer_height - KERNEL_VIRTUAL_BASE]
	add eax, [framebuffer_addr - KERNEL_VIRTUAL_BASE] ; eax = framebuffer end addr
	
	mov ecx, esi
	add ecx, [framebuffer_addr - KERNEL_VIRTUAL_BASE] ; ecx = framebuffer_addr + stride for one row of text
	.loop_start1:
		mov edx, ecx
		mov ebx, edx
		sub ebx, esi ; dest
		
		mov dl, [edx]
		mov byte [ebx], dl
		
		inc ecx
	cmp ecx, eax
	jl .loop_start1
	; ecx = end of framebuffer
	
	sub ecx, esi ; one row of text before end
	.loop_start2:
		mov byte [ecx], 0
		inc ecx
	cmp ecx, eax
	jl .loop_start2

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilogue
	mov esp, ebp
	pop ebp
	ret

extern psf_copychar
global _32_putc
_32_putc:
	; prologue
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi

	movzx ecx, byte [ebp+8]

	cmp cl, 0xa ; c == '\n'
	je .println
	cmp cl, 128 ; c == [col_reset]
	je .col_reset
	cmp cl, 129 ; c == [col_red]
	je .col_red
	cmp cl, 130 ; c == [col_green]
	je .col_green
		movzx eax, word [x - KERNEL_VIRTUAL_BASE]
		push eax
		movzx eax, word [y - KERNEL_VIRTUAL_BASE]
		push eax
		push ecx
		call psf_copychar
		add esp, 12
		
		inc word [x - KERNEL_VIRTUAL_BASE]
		jmp .epi
	.col_reset:
		mov dword [col - KERNEL_VIRTUAL_BASE], 0xffffff
		jmp .epi
	.col_green:
		mov dword [col - KERNEL_VIRTUAL_BASE], 0x00ff00
		jmp .epi
	.col_red:
		mov dword [col - KERNEL_VIRTUAL_BASE], 0xff0000
		jmp .epi
	.println:
		mov word [x - KERNEL_VIRTUAL_BASE], 0
		inc word [y - KERNEL_VIRTUAL_BASE]
		
		mov ax, [termsize_y - KERNEL_VIRTUAL_BASE]
		cmp word [y - KERNEL_VIRTUAL_BASE], ax
		jle .epi
			mov word [y - KERNEL_VIRTUAL_BASE], ax
			call _32_shift_up
	.epi:

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilogue
	mov esp, ebp
	pop ebp
	ret

global _32_puts
_32_puts:
	; prologue
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi

	mov esi, dword [ebp+8] ; esi = str

	.loop_cond:
		movzx ebx, byte [esi] ; bl = *str
		cmp ebx, 0
		jz .loop_end
		inc esi ; str++
	.loop_body:
		push ebx
		call _32_putc
		pop ebx
		jmp .loop_cond
	.loop_end:

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilogue
	mov esp, ebp
	pop ebp
	ret

extern psf_width
extern psf_height
extern framebuffer_width
extern framebuffer_height
global _32_init_io
_32_init_io: ; inits the termsize vars, parse_psf and read_multiboot_info must be called first
	; prologue
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi
	
	mov eax, [framebuffer_width - KERNEL_VIRTUAL_BASE]
	xor edx, edx
	mov ecx, [psf_width - KERNEL_VIRTUAL_BASE]
	inc ecx
	div dword ecx
	mov [termsize_x - KERNEL_VIRTUAL_BASE], eax
	
	mov eax, [framebuffer_height - KERNEL_VIRTUAL_BASE]
	xor edx, edx
	mov ecx, [psf_height - KERNEL_VIRTUAL_BASE]
	div dword ecx
	mov [termsize_y - KERNEL_VIRTUAL_BASE], eax
	
	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilogue
	mov esp, ebp
	pop ebp
	ret
