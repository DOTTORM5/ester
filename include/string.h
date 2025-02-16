#ifndef STRING_H
#define STRING_H

#include "types.h"

uint8_t strcmp(const char *str1, const char *str2);
uint32_t strlen(const char *str);
uint8_t  strcpy(const char *str_src, char *str_dst);
void  strrev(char *str); 
char * strncat (char * dst, const char * src, uint32_t n);

#endif