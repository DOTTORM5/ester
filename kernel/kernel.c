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

	ext2_dir_entry_fixed_name_t dir_entries[30];
	ext2_list_directory(2, dir_entries);

	ext2_file_t * file =  ext2_fopen("/prova", "w");

	uint8_t buffer[4096];
	memset(buffer, 0x61, 4096); 
	ext2_fwrite(file, buffer, 4096); 
	ext2_fseek(file, 10); 
	memset(&buffer, 0x62, 4096); 
	ext2_fwrite(file, buffer, 4096); 
	uint8_t read_buffer[4096];
	ext2_fread(file, read_buffer, 4096); 
	ext2_fclose(file); 
	
	printk("\n\n\n");
	printk("                                 Welcome to\n\n\n");
	printk("                              _______________\n");
	printk("                             |               |\n");
	printk("                             |  ESTER OS !!! |\n");
	printk("                             |_______________|\n\n\n\n");
	printk("                                                 \n");
	printk("                          Everything will be fine\n");


	while(1);

	return;
}
