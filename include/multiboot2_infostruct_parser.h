/* This module defines a parser for the multiboot2 info structure that the bootloader grub loads into memory along with other kernel module */
#ifndef MULTIBOOT2_INFOSTRUCT_PARSER_H
#define MULTIBOOT2_INFOSTRUCT_PARSER_H

#include "types.h"

#define MODULE_TAG_TYPE 3

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((aligned(0x8))) __fixed_tag;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    uint8_t * string;
} __attribute__((aligned(0x8))) __module_tag;


__module_tag * multiboot2_parser(uint32_t base_address); 



#endif // MULTIBOOT2_INFOSTRUCT_PARSER_H