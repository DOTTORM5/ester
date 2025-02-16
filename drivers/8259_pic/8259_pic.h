#ifndef _8259_PIC_H
#define _8259_PIC_H

#include "types.h"

void pic_disable(void);
void pic_init(void);
void pic_send_eoi(uint8_t irq);
uint16_t pic_get_isr(void);
uint16_t pic_get_irr(void);

#endif // _8259_PIC_H
