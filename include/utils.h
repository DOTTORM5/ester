#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* IO Utility Function */
/* Here I'm using extended assembly syntax */
/* Write a byte to port*/
static inline void outb ( __u16 port, __u8 val )
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    return;
}

/* Write a long word (32 bit) to port */
static inline void outl(__u16 port, __u32 val)
{
    __asm__ volatile ("outl %l0, %w1" : : "a"(val), "Nd"(port) : "memory");
    return;
}

/* Read a byte from port */
static inline __u8 inb  ( __u16 port )
{
    __u8 ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

/* Read a long word (32 bit) from port */
static inline __u32 inl  ( __u16 port )
{
    __u32 ret;
    __asm__ volatile ( "inl %w1, %l0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}


#endif // UTILS_H