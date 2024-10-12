#ifndef STRING_H
#define STRING_H

#include "types.h"

uint32_t strlen(const char *str);
uint8_t  strcpy(const char *str_src, char *str_dst);
void  strrev(char *str); 

#endif