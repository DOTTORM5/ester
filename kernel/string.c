#include "string.h"


__u32 strlen(const char *str)
{   
    __u32 len = 0;
    while(str[len]){
        len++;
    }
    return len;
}