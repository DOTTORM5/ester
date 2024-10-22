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
    //printk("VECTOR: %d\n", pic_get_isr());
    uint16_t vector = pic_get_isr();

    if (vector != 1 ) {
        printk("VECTOR: %d\n", pic_get_isr());
    }
    pic_send_eoi(vector-1);
    return;
}

/**
 * Function to set up idt descriptor
 * @param uint8_t vector: index in the vector table.
 * @param void* isr  : interrupt service routine.
 * @param uint8_t flags : general flags.
 * @return void
 */
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) 
{
    idt_entry_t* descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = (uint16_t)GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
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
    idtr.base = (uint64_t) &idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * (IDT_MAX_DESCRIPTORS - 1);
 
    for (uint8_t vector = 0; vector < 47; vector++) {
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
 * @param uint8_t  vector : interrupt vector
 * @param uint64_t addr   : interrupt handler address
 * @return void
 */
void register_interrupt_handler(uint8_t vector, uint64_t addr)
{
    idt_set_descriptor(vector, (void *)addr, 0x8E);
}