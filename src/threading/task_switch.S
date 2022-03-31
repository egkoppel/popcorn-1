bits 64

section .text
global task_switch
global task_init
extern current_task_ptr

; void task_init(void)
; takes 3 args, 1st arg in r13, 2nd arg in r14, 3rd arg in r15
task_init:
	mov rdi, r13
	mov rsi, r14
	mov rdx, r15
	ret

; void task_switch(Task *new_task)
; *new_task in rdi
task_switch:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	mov r11, current_task_ptr
	mov rsi, [r11]
	mov qword [rsi + 2*8], rsp ; save stack pointer

	; load new task
	mov [r11], rdi
	mov rsp, qword [rdi + 2*8] ; restore stack pointer
	mov rax, qword [rdi + 3*8] ; load new cr3 ito rax
	mov rcx, cr3 ; load old cr3 into rcx

	cmp rcx, rax ; check if tlb flush needed
	je .after_addr_space_switch
	mov cr3, rax ; set new cr3
.after_addr_space_switch:
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret