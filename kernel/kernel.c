#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "8259_pic.h"


	__attribute__((aligned(0x8))) 
	__u32 * m2_info;

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

	// __asm__ volatile ("int $0");
	m2_info = ( __u32 * ) m2_info_address;
	// if (  m2_info_address ==  0x107f38 ) {
	if ( *m2_info == 0x410 ) {
		DEBUG_INFO(" > 32");
	}
	else {
		DEBUG_INFO(" < 32");
	}

	return; 
}	
