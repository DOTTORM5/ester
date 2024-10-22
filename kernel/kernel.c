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
#include "mem.h"

#define AHCI_IE 0x00000002


static char buff[1024] __attribute__((aligned(0x1000)));
static char buff_r[1024] __attribute__((aligned(0x1000)));

void kernel_entry( uint32_t m2_info_address )
{
	__asm__ volatile ("cli");

	idt_init();


	pci_recursive_scan(); /* Start PCI scanning */

	ps2_keyboard_init();
	pic_init();

	__asm__ volatile ("sti");

	HBA_MEM * hba_mem_ptr;
	hba_mem_ptr = (HBA_MEM *) pci_ahci_get_abar();


	hba_mem_ptr->ghc |= AHCI_GHC_HR; /* Reset the controller */
	while (hba_mem_ptr->ghc & AHCI_GHC_HR); /* Wait the reset complention */

	hba_mem_ptr->ghc |= (AHCI_GHC_AE | AHCI_IE);

	ahci_probe_port(hba_mem_ptr);

	for (uint16_t i = 0; i < 1024; i++) {
		buff[i] = 0x61;
	}

	port_rebase(&hba_mem_ptr->ports[0], 0);

	write(&hba_mem_ptr->ports[0], 0, 0, 1, buff);

	read(&hba_mem_ptr->ports[0], 0, 0, 1, buff_r);
	// read(&hba_mem_ptr->ports[0], 0, 0, 1, buff_r);
	// read(&hba_mem_ptr->ports[0], 0, 0, 1, buff_r);
	// read(&hba_mem_ptr->ports[0], 0, 0, 1, buff_r);

	uint32_t i = 0;
	// while(i++ < 100000000);

	for (uint16_t i = 0; i < 64; i++) {
		printk("%s", buff_r[i]);
	}

	
	while(1);

	return;
}
