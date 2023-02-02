%include "initialisation/tss.inc"

bits 64

section .text

extern task_startup_scheduler_unlock

; ===================================================================================
; void task_init()
;
; @brief Entry function to be run on task startup
; @attention Does not use standard System V calling convention
; @param [rbx] Argument to the task
; @param [rbp] Argument to the task
; @param [r12] Argument to the task
; @param [r13] Argument to the task
; @param [r14] Argument to the task
; @param [r15] Argument to the task
;
; ===================================================================================
    global task_startup
task_startup:
    call task_startup_scheduler_unlock
	mov rdi, r15
	mov rsi, r14
	mov rdx, r13
	mov rcx, r12
	mov r8, rbp
	mov r9, rbx
	xor rbp, rbp
	ret

extern task_stack_ptr_storage
extern get_kstack_top
extern get_p4_table_frame
extern task_state_segment

; ===================================================================================
; void task_switch_asm(threads::Task *new_task, threads::Task *old_task)
;
; @brief Context switches from old_task to new_task, storing register and stack state
; @param new_task [rdi] Pointer to the task to switch to
; @param old_task [rsi] Pointer to the current task
;
; @local [r12] new_task
; @local [r13] old_task
; ===================================================================================
    global task_switch_asm
task_switch_asm:
    ; save current register state
	push rbp
	push rbx
	push r12
	push r13
	push r14
	push r15

    ; @locals
    mov r12, rdi
    mov r13, rsi

    ; save stack pointer
    mov rdi, r13
    call task_stack_ptr_storage
	mov qword [rax], rsp

    ; load stack pointer
    mov rdi, r12
    call task_stack_ptr_storage
    mov rsp, qword [rax]

    ; load tss rsp0
    mov rdi, r12
    call get_kstack_top ; get top of kernel stack
    mov qword [task_state_segment + tss.rsp0], rax ; save into TSS

    mov rdi, r12
    call get_p4_table_frame ; loads new cr3 ito rax
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
