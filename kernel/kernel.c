#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "8259_pic.h"
#include "utils.h"
#include "pci.h"
#include "string.h"
#include "printk.h"
#include "ps2_keyboard.h"
#include "ahci.h"
#include "mem.h"
#include "block_device.h"
#include "ext2.h"
#include "elf64_loader.h"

void kernel_entry( uint32_t m2_info_address )
{
	__asm__ volatile ("sti");
	// __asm__ volatile ("cli");

	idt_init();


	pci_recursive_scan(); /* Start PCI scanning */

	ps2_keyboard_init();
	pic_init();

	// __asm__ volatile ("sti");


	ahci_init();
	init_block_device();


	ext2_extract_sb();
	ext2_extract_bgdt();

	// ext2_list_directory(2);
	ext2_init_cwd(); 
	ext2_change_cwd("/");

	elf64_load("elf_example");

	while(1);

	return;
}
