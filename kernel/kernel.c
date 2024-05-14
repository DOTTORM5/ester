#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"
#include "8259_pic.h"

void kernel_entry()
{
	 __asm__ volatile ("sti");
	DEBUG_INFO("KERNEL ENTRY!!!");

	__asm__ volatile ("cli");

	/* Disable the PIC, we want to use the APIC */
	DEBUG_INFO("Disabling the PIC");
	pic_disable();

	idt_init();

	DEBUG_INFO("IDT LOADED AND INTERRUPTS ENABLED");

	__asm__ volatile ("int $0");

	return; 
}	
