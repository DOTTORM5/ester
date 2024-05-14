#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* IO Utility Function */

/* Write a byte to port*/
static inline void outb ( __u16 port, __u8 val )
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    return;
}

/* Read a byte from port */
static inline __u8 inb  ( __u16 port )
{
    __u8 ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}


#endif // UTILS_H