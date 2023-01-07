bits 64

section .text

; ===================================================================================
; void switch_to_user_mode()
;
; @brief
;
; ===================================================================================
    global switch_to_user_mode
switch_to_user_mode:
    ; TODO: swapgs ; switch to user gs data
	pop rax ; get return address
	mov r11, rsp ; save current stack pointer

	push (0x20 | 3) ; entry 4 -> 0x20 (user data), ring 3
	push r11 ; return stack pointer
	pushfq ; flags
	push (0x28 | 3) ; entry 5 -> 0x28 (user code long mode), ring 3
	push rax ; return address
	iretq