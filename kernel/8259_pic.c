#include "utils.h"
#include "8259_pic.h"


#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

/* Helper func */
static __u16 __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_CMD, ocw3);
    outb(PIC2_CMD, ocw3);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
__u16 pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
__u16 pic_get_isr(void)
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
	/* Set cascading mode */
	outb(PIC1_CMD, 0x10 + 0x01);
	outb(PIC2_CMD,  0x10 + 0x01);
	/* Setup master's vector offset */
	outb(PIC1_DATA, 0x20);
	/* Tell the slave its vector offset */
	outb(PIC2_DATA, 0x28);
	/* Tell the master that he has a slave */
	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);
	/* Enabled 8086 mode */
	outb(PIC1_DATA, 0x01);
	outb(PIC2_DATA, 0x01);

	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
    return;
}

void pic_send_eoi(__u8 irq)
{
	if(irq >= 8)
		outb(PIC2_CMD, PIC_EOI_CMD);
	outb(PIC1_CMD, PIC_EOI_CMD);
}