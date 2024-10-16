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
#include "ps2_keyboard.h"
#include "ahci.h"

void kernel_entry( uint32_t m2_info_address )
{
	__asm__ volatile ("cli");
	// DEBUG_INFO("KERNEL ENTRY!!!");
	// DEBUG_INFO("Init the PIC in cascade mode");

	idt_init();

	// DEBUG_INFO("IDT LOADED AND INTERRUPTS ENABLED");

	// multiboot2_parser(m2_info_address);

	pci_recursive_scan(); /* Start PCI scanning */

	ps2_keyboard_init();
	pic_init();

	
	// uint32_t ahci_abar = pci_ahci_get_abar();
	
	// printk("ABAR: %d\n", ahci_abar);

	HBA_MEM * hba_mem_ptr;
	hba_mem_ptr = (HBA_MEM *) pci_ahci_get_abar();

	hba_mem_ptr->ghc |= 0x80000000;

	ahci_probe_port(hba_mem_ptr);

	__asm__ volatile ("sti");
	while(1);

	return;
}
