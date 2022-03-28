bits 64

section .text
global switch_to_user_mode

switch_to_user_mode:
	pop rax ; get return address
	mov r11, rsp ; save current stack pointer

	push (0x20 | 3) ; entry 0x20 (user data), ring 3
	push r11 ; return stack pointer
	pushfq ; flags
	push (0x28 | 3) ; entry 0x20 (user code), ring 3
	push rax ; return address
	iretq