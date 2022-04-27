bits 64

section .text
global task_switch_asm
global task_init
extern unlock_scheduler_from_task_init
extern task_state_segment

; void task_init(void)
; takes 6 args in rbx,rbp,r12,r13,r14,r15
task_init:
	call unlock_scheduler_from_task_init

	mov rdi, rbx
	mov rsi, rbp
	mov rdx, r12
	mov rcx, r13
	mov r8, r14
	mov r9, r15
	xor rbp, rbp
	ret

; void task_switch_asm(Task *new_task, Task *old_task)
; *new_task in rdi, *old_task in rsi
task_switch_asm:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	mov qword [rsi + 4*8], rsp ; save stack pointer

	; load new task
	mov rsp, qword [rdi + 4*8] ; restore stack pointer

	mov rax, task_state_segment ; move tss ptr into rax
	mov r11, qword [rdi + 2*8] ; load kernel stack top into r11
	mov [rax + 4], r11 ; load kernel stack top into TSS PL change stack table

	mov rax, qword [rdi + 5*8] ; load new cr3 ito rax
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