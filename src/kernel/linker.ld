ENTRY(_start)

SECTIONS {
    . = 0x8000;
    .ap_bootstrap.text ALIGN(4K) :
    {
        */ap_bootstrap*.o(.text.trampoline)
        . = ALIGN(8);
        *(.multiboot2)
        */ap_bootstrap*.o(.text*)
    }
    .ap_bootstrap.data ALIGN(4K) :
    {
        */ap_bootstrap*.o(.data*)
    }

	KERNEL_OFFSET = 0xFFFF800000000000;
	. = 1M + KERNEL_OFFSET;

	.kernelspace.text ALIGN(4K) : AT(ADDR(.kernelspace.text) - KERNEL_OFFSET)
	{
		*(EXCLUDE_FILE(*userspace/*.o */ap_bootstrap*.o) .text*)
	}
 
	/* Read-only data. */
	.kernelspace.rodata ALIGN(4K) : AT(ADDR(.kernelspace.rodata) - KERNEL_OFFSET)
	{
		ALIGN(64)
		preinit_array_start = .;
		*(SORT(.preinit_array*))
		preinit_array_end = .;

		ALIGN(64)
		init_array_start = .;
		*(SORT(.init_array*))
		init_array_end = .;

		ALIGN(64)
		start_ctors = .;
        *(SORT(.ctors*))
        end_ctors = .;

		ALIGN(64)
        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

		*(EXCLUDE_FILE(*userspace/*.o */ap_bootstrap*.o) .rodata*)
		*(.eh_frame*)
	}
 
	/* Read-write data (initialized) */
	.kernelspace.data ALIGN(4K) : AT(ADDR(.kernelspace.data) - KERNEL_OFFSET)
	{
		*(EXCLUDE_FILE(*userspace/*.o */ap_bootstrap*.o) .data*)
		*(EXCLUDE_FILE(*userspace/*.o */ap_bootstrap*.o) .got*)
	}
 
	/* Read-write data (uninitialized) and stack */
	.kernelspace.bss ALIGN(4K) : AT(ADDR(.kernelspace.bss) - KERNEL_OFFSET)
	{
		*(COMMON)
		*(EXCLUDE_FILE(*userspace/*.o */ap_bootstrap*.o) .bss*)
	}

	.userspace.text ALIGN(4K) : AT(ADDR(.userspace.text) - KERNEL_OFFSET)
	{
	    *userspace/*.o(.text*)
	}

	.userspace.rodata ALIGN(4K) : AT(ADDR(.userspace.rodata) - KERNEL_OFFSET)
    {
        *userspace/*.o(.rodata*)
    }

	.userspace.data ALIGN(4K) : AT(ADDR(.userspace.data) - KERNEL_OFFSET)
    {
        *userspace/*.o(.data*)
    }

    .userspace.bss ALIGN(4K) : AT(ADDR(.userspace.bss) - KERNEL_OFFSET)
    {
        *userspace/*.o(COMMON)
        *userspace/*.o(.bss*)
    }

	/DISCARD/ : {
		*(.debug_gdb_scripts*)
	}
}