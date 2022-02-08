bits 32

section .bss
global x
global y
x: resb 1
y: resb 1
itoa_buffer: resb 32

section .rodata
itoa_str db "0123456789abcdefghijklmnopqrstuvwxyz"

section .data
global col
col db 0x0f

section .text
global _32_utoa
_32_utoa:
	; prolog
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

		movzx ecx, byte [itoa_str + edx] ; cl = itoa_str[val % base]
		mov byte [itoa_buffer + edi], cl ; buf[i] = cl = itoa_str[val % base]

		dec edi ; --i
		; val /= base already done since quotient in eax

		cmp eax, 0 ; if (val == 0) break;
		je .loop_end

		cmp edi, 0 ; if (i < 0) break;
		jl .loop_end
	jmp .loop_start
	.loop_end:

	lea eax, [itoa_buffer + edi + 1]; return &buf[i+1]

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilog
	mov esp, ebp
	pop ebp
	ret

global _32_shift_up
_32_shift_up:
	; prolog
	push ebp
	mov ebp, esp

	; save regs
	push ebx
	push esi
	push edi

	mov ecx, 0
	.loop_start1:
		movzx eax, word [0xB8000 + ecx*2]
		lea ebx, [0xB8000 + ecx*2 + 80]
		mov word [ebx], ax

		inc ecx
	cmp ecx, 80*25
	jl .loop_start1

	mov ecx, 0
	.loop_start2:
		lea eax, [0xB8000 + ecx*2 + 80*24*2]
		mov word [eax], 0x0F20 ; white on black space
		inc ecx
	cmp ecx, 80
	jl .loop_start2

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilog
	mov esp, ebp
	pop ebp
	ret

global _32_putc
_32_putc:
	; prolog
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
		movzx eax, byte [y] ; get current y
		mov edx, 80
		mul byte dl ; ax = y*80

		movzx edx, byte [x]
		add eax, edx ; eax = y*80 + x

		lea ebx, [0xB8000 + 2*eax] ; eax = 0xB8000 + 2*(x + 80*y)

		; upper bits already cleared by movzx before loop so can write to ch
		mov ch, [col] ; ch:cl = col:char
		mov word [ebx], cx

		inc byte [x]
		jmp .epi
	.col_reset:
		mov byte [col], 0x0f
		jmp .epi
	.col_green:
		mov byte [col], 0x02
		jmp .epi
	.col_red:
		mov byte [col], 0x04
		jmp .epi
	.println:
		mov byte [x], 0
		inc byte [y]

		cmp byte [y], 24
		jle .epi
			mov byte [y], 24
			call _32_shift_up
	.epi:

	; unsave regs
	pop edi
	pop esi
	pop ebx

	; epilog
	mov esp, ebp
	pop ebp
	ret

global _32_puts
_32_puts:
	; prolog
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

	; epilog
	mov esp, ebp
	pop ebp
	ret