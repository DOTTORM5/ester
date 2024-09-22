#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "8259_pic.h"
#include "utils.h"
// #include "multiboot2_infostruct_parser.h"
#include "pci.h"


void kernel_entry( __u32 m2_info_address )
{
	 __asm__ volatile ("sti");
	DEBUG_INFO("KERNEL ENTRY!!!");

	__asm__ volatile ("cli");

	/* Disable the PIC, we want to use the APIC */
	DEBUG_INFO("Disabling the PIC");
	pic_disable();

	idt_init();

	DEBUG_INFO("IDT LOADED AND INTERRUPTS ENABLED");

	// multiboot2_parser(m2_info_address);

	pci_recursive_scan(); /* Start PCI scanning */


	__u8 scan_code = inb(0x60);
	outb(0x64, 0xF5);
	while ( inb(0x60) != 0xFA );

	/* PS/2 keyboard */
	// outb(0x64, 0xF0);
	// outb(0x60, 0x01);

	// while ( inb(0x60) != 0xFA );

	// outb(0x64, 0xF0);
	// outb(0x60, 0x02);

	// while ( inb(0x60) != 0xFA );

	// outb(0x64, 0xF0);
	// outb(0x60, 0x03);

	// while ( inb(0x60) != 0xFA );

	DEBUG_INFO("FINISH");


	return;
}
