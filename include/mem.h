#ifndef MEM_H
#define MEM_H
#include "types.h"

void memcpy(void * dst, const void * src, uint32_t len); 
void memset(void * ptr, uint8_t val, uint32_t len);
void map_page(uint64_t phys_addr, uint64_t virt_addr, uint64_t flags);


#endif