bits 64
section .text
extern main
global _start
_start:
    mov rax, 0x100 ; make_stack
    syscall
    cmp rax, 0
    jle .die
    mov rsp, rax ; load new stack ptr

    call main ; call main()
    mov rdi, rax ; move return code into syscall arg 1
    jmp .exit

    .die:
    mov rdi, -1
    .exit:
    mov rax, 0x002 ; exit
    syscall
    ud2
