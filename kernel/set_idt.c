#include "idt.h"
#include "int_handler.h"
#include "kernel.h"
#include "debug.h"
#include "utils.h"
#include "8259_pic.h"
#include "printk.h"

extern void* isr_stub_table[];

static idtr_t idtr;

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance


// This is a general interrupt/exception handler, for now just a stub // to put in another specific file
// __attribute__((noreturn))
void exception_handler(void) 
{
    // DEBUG_WARNING("INTO THE HANDLER");
    // printk("VECTOR: %d\n", vector);
    // printk("VECTOR: %d\n", pic_get_isr());
    __u16 vector = pic_get_isr();
    pic_send_eoi(vector-1);
    return;
}

/**
 * Function to set up idt descriptor
 * @param __u8 vector: index in the vector table.
 * @param void* isr  : interrupt service routine.
 * @param __u8 flags : general flags.
 * @return void
 */
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
    return;
}

/**
 * Function to set up IDT
 * @param void
 * @return void
 */
void idt_init() 
{
    idtr.base = (__u64) &idt[0];
    idtr.limit = (__u16)sizeof(idt_entry_t) * (IDT_MAX_DESCRIPTORS - 1);
 
    for (__u8 vector = 0; vector < 47; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E); // 0x8E is for interrupt gate, 0x8F is for trap gate etc...
        // vectors[vector] = true;
    }
 
    __asm__ volatile ("lidt [idtr]"); // load the new IDT
    // __asm__ volatile ("sti"); // set the interrupt flag
    return; 
}

/* We use idt_set_descriptor avoiding to enter the stub table and call exception_handler we have to think about this... */
/**
 * Function to register interrupt handler
 * @param __u8  vector : interrupt vector
 * @param __u64 addr   : interrupt handler address
 * @return void
 */
void register_interrupt_handler(__u8 vector, __u64 addr)
{
    idt_set_descriptor(vector, (void *)addr, 0x8E);
}