ENTRY(_start)

SECTIONS {
	KERNEL_OFFSET = 0xFFFFFF8000000000;
	. = 1M + KERNEL_OFFSET;

	.kernelspace.text ALIGN(4K) : AT(ADDR(.kernelspace.text) - KERNEL_OFFSET)
	{
		*(.multiboot2)
		*(EXCLUDE_FILE(*userspace/*.o).text*)
	}
 
	/* Read-only data. */
	.kernelspace.rodata ALIGN(4K) : AT(ADDR(.kernelspace.rodata) - KERNEL_OFFSET)
	{
		ALIGN(64)
		preinit_array_start = .;
		*(SORT(EXCLUDE_FILE(*userspace/*.o).preinit_array*))
		preinit_array_end = .;

		ALIGN(64)
		init_array_start = .;
		*(SORT(EXCLUDE_FILE(*userspace/*.o).init_array*))
		init_array_end = .;

		ALIGN(64)
		start_ctors = .;
        *(SORT(EXCLUDE_FILE(*userspace/*.o).ctors*))
        end_ctors = .;

		ALIGN(64)
        start_dtors = .;
        *(SORT(EXCLUDE_FILE(*userspace/*.o).dtors*))
        end_dtors = .;

		*(EXCLUDE_FILE(*userspace/*.o).rodata*)
		*(.eh_frame*)
	}
 
	/* Read-write data (initialized) */
	.kernelspace.data ALIGN(4K) : AT(ADDR(.kernelspace.data) - KERNEL_OFFSET)
	{
		*(EXCLUDE_FILE(*userspace/*.o).data*)
		*(EXCLUDE_FILE(*userspace/*.o).got*)
	}
 
	/* Read-write data (uninitialized) and stack */
	.kernelspace.bss ALIGN(4K) : AT(ADDR(.kernelspace.bss) - KERNEL_OFFSET)
	{
		*(COMMON)
		*(EXCLUDE_FILE(*userspace/*.o).bss*)
	}

	.userspace.text ALIGN(4K) : AT(ADDR(.userspace.text) - KERNEL_OFFSET)
	{
	    *(.text*)
	}

	.userspace.rodata ALIGN(4K) : AT(ADDR(.userspace.rodata) - KERNEL_OFFSET)
    {
        *(.rodata*)
    }

	.userspace.data ALIGN(4K) : AT(ADDR(.userspace.data) - KERNEL_OFFSET)
    {
        *(.data*)
    }

	/DISCARD/ : {
		*(.debug_gdb_scripts*)
	}
}