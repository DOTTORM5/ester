#ifndef _8259_PIC_H
#define _8259_PIC_H

#include "types.h"

#define PIC1		    0x20		/* IO base address for master PIC */
#define PIC2		    0xA0		/* IO base address for slave PIC */
#define PIC1_CMD    	0x20
#define PIC1_DATA	    0x21
#define PIC2_CMD    	0xA0
#define PIC2_DATA	    0xA1
#define PIC_EOI_CMD     0x20

void pic_disable(void);
void pic_init(void);
void pic_send_eoi(__u8 irq);
__u16 pic_get_isr(void);

#endif // _8259_PIC_H
