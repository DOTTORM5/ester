#ifndef STRING_H
#define STRING_H

#include "types.h"

__u32 strlen(const char *str);
__u8  strcpy(const char *str_src, char *str_dst);
void  strrev(char *str); 

#endif