ENTRY(_start)

SECTIONS {
	. = 0xC0100000;
 
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot2)
		*(.text)
	}
 
	/* Read-only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		ALIGN(64)
		preinit_array_start = .;
		*(SORT(.preinit_array))
		preinit_array_end = .;

		ALIGN(64)
		init_array_start = .;
		*(SORT(.init_array))
		init_array_end = .;

		ALIGN(64)
		start_ctors = .;
        *(SORT(.ctors))
        end_ctors = .;

		ALIGN(64)
        start_dtors = .;
        *(SORT(.dtors))
        end_dtors = .;

		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}
}