ENTRY(_start)

SECTIONS {
	KERNEL_OFFSET = 0xFFFFFF8000000000;
	. = 1M + KERNEL_OFFSET;
 
	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_OFFSET)
	{
		*(.multiboot2)
		*(.text*)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_OFFSET)
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

		*(.rodata*)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_OFFSET)
	{
		*(.data*)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_OFFSET)
	{
		*(COMMON)
		*(.bss*)
	}

	/DISCARD/ : {
		*(.debug_gdb_scripts*)
	}
}