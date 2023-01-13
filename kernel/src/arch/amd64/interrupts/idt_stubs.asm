bits 64

section .text
%macro isr_err_stub 1
isr_stub_%+%1:
    push qword %1
    jmp amd64_exception_handler_entry
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push qword -1
    push qword %1
    jmp amd64_exception_handler_entry
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

%assign i 32
%rep    224
isr_no_err_stub i
%assign i i+1
%endrep

extern exception_handler_entry

amd64_exception_handler_entry:
    push qword 0 ; for cr2 to go into
    push qword rax
    push qword rcx
    push qword rdx
    push qword rsi
    push qword rdi
    push qword r8
    push qword r9
    push qword r10
    push qword r11

    ; locate stack frame
    mov rdi, rsp
    add rdi, 9*8 ; load stack frame as first arg
    mov rsi, cr2 ; load cr2 value
    mov [rdi], rsi

    ; realign stack
    sub rsp, 8

    ; call kernel exception handler
    call exception_handler_entry

    ; reset stack
    add rsp, 8

    pop qword r11
    pop qword r10
    pop qword r9
    pop qword r8
    pop qword rdi
    pop qword rsi
    pop qword rdx
    pop qword rcx
    pop qword rax
    add rsp, 24 ; get rid of error code, vector number and cr2
    iretq

section .data

    global isr_stub_table
    align 8
isr_stub_table:
%assign i 0
%rep    256
    dq isr_stub_%+i
%assign i i+1
%endrep
