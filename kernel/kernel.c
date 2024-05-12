#include "types.h"
#include "debug.h"
#include "idt.h"
#include "kernel.h"

void kernel_entry()
{
	 __asm__ volatile ("sti");
	DEBUG_INFO("KERNEL ENTRY!!!");

	 __asm__ volatile ("cli");
	idt_init();

	DEBUG_INFO("IDT LOADED");

	// __asm__ volatile ("sti");

	__u32 i = 0;

	for ( i = 0; i < 1000000; i++);
    __asm__ volatile ("sti");


	return; 
}	
