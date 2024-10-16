/* entry point of our kernel */
ENTRY(start)

SECTIONS
{
	/* we need 1MB of space atleast (for the DEVICE? for what?) */
	. = 1M;

  	/* text section */
	.text BLOCK(16K) : ALIGN(4K)
	{
		*(.multiboot_header)
		*(.text)
	}

	/* read only data section */
	/*.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}*/

	/* data section */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* bss section */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}