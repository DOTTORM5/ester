#include "utils.h"
#include "8259_pic.h"

#define PIC1		    0x20		/* IO base address for master PIC */
#define PIC2		    0xA0		/* IO base address for slave PIC */
#define PIC1_CMD    	0x20
#define PIC1_DATA	    0x21
#define PIC2_CMD    	0xA0
#define PIC2_DATA	    0xA1
#define PIC_EOI_CMD     0x20

#define PIC_READ_IRR    0x0a        /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR    0x0b        /* OCW3 irq service next CMD read */
#define ICW1_ICW4	    0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	    0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	    0x10		/* Initialization - required! */

#define ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	    0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	    0x10		/* Special fully nested (not) */

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_CMD, ocw3);
    outb(PIC2_CMD, ocw3);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

/* Disable the PIC */
void pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

/* Initialize the pic */
void pic_init(void)
{
	uint8_t a1, a2;
	/* Save masks */
	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

	/* Set cascading mode */
	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	/* Setup master's vector offset */
	outb(PIC1_DATA, 0x20);
	/* Tell the slave its vector offset */
	outb(PIC2_DATA, 0x28);
	/* Tell the master that he has a slave */
	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);
	/* Enabled 8086 mode */
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

	/* Restore saved masks */
	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
    return;
}

/* Send end of interrupt to the PIC - close the interrupt - ack the interrupt */
void pic_send_eoi(uint8_t irq)
{
	if(irq >= 8)
		outb(PIC2_CMD, PIC_EOI_CMD);
	outb(PIC1_CMD, PIC_EOI_CMD);
}