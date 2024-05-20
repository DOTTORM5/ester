#ifndef TYPES_H
#define TYPES_H

typedef unsigned long long  __u64;
typedef unsigned int        __u32;   /* For some forget reason, I used -std=gnu99 as standard C (maybe some guide tell me this), in this standard unsigned long is 64 bit, so i have to use unsigned int */
typedef unsigned short      __u16;
typedef unsigned char       __u8;    /* This is very ugly, but works */

#endif