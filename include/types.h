#ifndef TYPES_H
#define TYPES_H

typedef unsigned long long  uint64_t;
typedef unsigned int        uint32_t;   /* For some forget reason, I used -std=gnu99 as standard C (maybe some guide tell me this), in this standard unsigned long is 64 bit, so i have to use unsigned int */
typedef unsigned short      uint16_t;
typedef unsigned char       uint8_t;    /* This is very ugly, but works */

#endif