#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "8259_pic.h"
#include "utils.h"
// #include "multiboot2_infostruct_parser.h"
#include "pci.h"
#include "string.h"
#include "printk.h"


static int ps2_wait_input(void) {
	__u32 timeout = 100000;
	while (--timeout) {
		if (!(inb(0x64) & (1 << 1))) return 0;
	}
	return 1;
}

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


	// __u8 scan_code = inb(0x64);
	
	// ps2_wait_input();

	// // disable ports
	// outb(0x64, 0xAD); // port1

	// ps2_wait_input();

	// outb(0x64, 0xAD); // port2

	/* Clear the input buffer. */
	// while ((inb(0x64) & 1)) inb(0x60);

	// enable ports
	// outb(0x64, 0xAE); // port1

	// ps2_wait_input();

	// outb(0x64, 0xA8); // port2


	// __u8 scan_code;
	// // // outb(0x64, 0xA7);
	// // scan_code = inb(0x60);

	// while(1){
	// 	// if (!(inb(0x64) & (1 << 1))) {
	// 		scan_code = inb(0x60);
	// 		if (scan_code == 0x20) {
	// 			DEBUG_INFO("Something pressed!");
	// 		}
	// 	// }
	// }

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

	

	printk("%d %d %d %d", 1, 2, 3, 4, 5, 6, 7);

	DEBUG_INFO("FINISH");


	return;
}
