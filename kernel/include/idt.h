#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_MAX_DESCRIPTORS 256

/* Interrupt descriptor table */
/* IDT Entry 64 bit*/
typedef struct {
	__u16    isr_low;      // The lower 16 bits of the ISR's address
	__u16    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	__u8	 ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	__u8     attributes;   // Type and attributes; see the IDT page
	__u16    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	__u32    isr_high;     // The higher 32 bits of the ISR's address
	__u32    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

/* IDT descriptor for IDTR (just like GDT descriptor for GDTR) */
typedef struct {
	__u16	limit;
	__u64	base;
} __attribute__((packed)) idtr_t;


/**
 * Function to set up idt descriptor
 * @param __u8 vector: index in the vector table.
 * @param void* isr  : interrupt service routine.
 * @param __u8 flags : general flags.
 * @return void 
 */
void idt_set_descriptor(__u8 vector, void* isr, __u8 flags);

/**
 * Function to set up IDT
 * @param void
 * @return void 
 */
void idt_init(void);


#endif // IDT_H