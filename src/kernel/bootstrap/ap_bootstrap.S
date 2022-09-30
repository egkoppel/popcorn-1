bits 16

section .data
ap_running_count: db 0
ap_wait_flag: db 0

section .text
global ap_trampoline
global ap_running_count
global ap_wait_flag
ap_trampoline:
.1: pause
    cmp byte [ap_wait_flag], 0
    jz .1

    lock inc byte [ap_running_count]
.2: jmp .2