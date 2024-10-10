#ifndef PRINTK_H
#define PRINTK_H

#define INT_MAX_DIGITS 16 // this has to be 8 or even 16 byte aligned, or simple it is an eaven number, it is we have to pass to dynamic allocation

void printk(const char *str, ...);

#endif