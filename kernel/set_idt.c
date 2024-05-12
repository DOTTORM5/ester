#include "idt.h"
#include "int_handler.h"
#include "kernel.h"
#include "debug.h"

extern void* isr_stub_table[];

static idtr_t idtr;

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance



// This is a general interrupt/exception handler, for now just a stub // to put in another specific file
// __attribute__((noreturn))
void exception_handler() 
{
    DEBUG_WARNING("INTO THE HANDLER"); 
    __asm__ volatile ("cli; hlt");    
}

void idt_set_descriptor(__u8 vector, void* isr, __u8 flags) 
{
    idt_entry_t* descriptor = &idt[vector];
 
    descriptor->isr_low        = (__u64)isr & 0xFFFF;
    descriptor->kernel_cs      = (__u16)GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((__u64)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((__u64)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

 
void idt_init() 
{
    idtr.base = (__u64) &idt[0];
    idtr.limit = (__u16)sizeof(idt_entry_t) * (IDT_MAX_DESCRIPTORS - 1);
 
    for (__u8 vector = 0; vector < 50; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        // vectors[vector] = true;
    }
 
    // __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("lidt [idtr]");
    // __asm__ volatile ("sti"); // set the interrupt flag
}