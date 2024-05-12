#include "types.h"

int strlen(char *str)
{   
    __u32 len = 0;
    while(str[len]){
        len++;
    }
    return len;
}