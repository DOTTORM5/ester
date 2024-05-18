/* This module defines a parser for the multiboot2 info structure that the bootloader grub loads into memory along with other kernel module */
#ifndef MULTIBOOT2_INFOSTRUCT_PARSER_H
#define MULTIBOOT2_INFOSTRUCT_PARSER_H

#include "types.h"

#define MODULE_TAG_TYPE 3

typedef struct {
    __u32 total_size;
    __u32 reserved;
} __attribute__((aligned(0x8))) __fixed_tag;

typedef struct {
    __u32 type;
    __u32 size;
    __u32 mod_start;
    __u32 mod_end;
    __u8 * string;
} __attribute__((aligned(0x8))) __module_tag;


void multiboot2_parser(__u32 base_address); 



#endif // MULTIBOOT2_INFOSTRUCT_PARSER_H