amd64
=====

Initial physical memory layout
------------------------------

================ == ========= === ===========================
start               end
================ == ========= === ===========================
0x8000              0x100000  1M  Bootstrap for real mode APs
0x100000         1M <0x200000 <2M Core kernel image
0x200000         2M 0x600000  6M  Initial `mem_map` region
GRUB controlled                   Multiboot info struct
GRUB controlled                   Ramdisk and kernel modules
================ == ========= === ===========================
