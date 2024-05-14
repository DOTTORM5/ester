#include "utils.h"
#include "8259_pic.h"


/* Disable the PIC */
void pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}