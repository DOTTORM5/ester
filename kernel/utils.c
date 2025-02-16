#include "utils.h"

/* IO Utility Function */

// /* Write a byte to port*/
// static inline void outb ( uint16_t port, uint8_t val )
// {
//     __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
//     return;
// }

// /* Read a byte from port */
// static inline uint8_t inb  ( uint16_t port )
// {
//     uint8_t ret;
//     __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
//     return ret;
// }