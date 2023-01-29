amd64
=====

Initial physical memory layout
------------------------------

================ == ========= === ===========================
start               end
================ == ========= === ===========================
0x8000              0x100000  1M  Bootstrap for real mode APs
0x200000         2M 0x600000  6M  Initial `mem_map` region
0x600000         6M <0x700000 <7M Core kernel image
GRUB controlled                   Multiboot info struct
GRUB controlled                   Ramdisk and kernel modules
================ == ========= === ===========================

Syscall calling convention
--------------------------

Syscalls are called through the `syscall` instruction. The syscall number is passed in `rax`. Arguments are passed in `rdi`, `rsi`, `rdx`, `r8`, `r9`. `rcx`, `r11` and `r12` are clobbered. Return value is passed back in `rax`.