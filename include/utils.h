#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* IO Utility Function */
/* Here I'm using extended assembly syntax */
/* Write a byte to port*/
static inline void outb ( uint16_t port, uint8_t val )
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    return;
}

/* Write a long word (32 bit) to port */
static inline void outl(uint16_t port, uint32_t val)
{
    // __asm__ volatile ("outl %l0, %w1" : : "a"(val), "Nd"(port) : "memory");
    __asm__ volatile ("outl %0, %w1" : : "a"(val), "Nd"(port) : "memory");
    return;
}

/* Read a byte from port */
static inline uint8_t inb  ( uint16_t port )
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

/* Read a long word (32 bit) from port */
static inline uint32_t inl  ( uint16_t port )
{
    uint32_t ret;
    // __asm__ volatile ( "inl %w1, %l0" : "=a"(ret) : "Nd"(port) : "memory");
    __asm__ volatile ( "inl %w1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}


#endif // UTILS_H