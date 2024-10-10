#include "string.h"
#include "debug.h"
#include "vga.h"

/* String length */
__u32 strlen(const char *str)
{   
    __u32 len = 0;
    while(str[len]){
        len++;
    }
    return len;
}

/* Sting copy - we need to introduce ERROR CODES*/
__u8 strcpy(const char *str_src, char *str_dst)
{
    __u32 src_len = strlen(str_src);
    __u32 dst_len = strlen(str_dst);

    if (src_len != dst_len) return 1; 

    for ( __u32 i = 0; i < dst_len; i++ ) {
        str_dst[i] = str_src[i];
    }

    return 0;
}

/* String reverse */
void strrev (char *str) 
{
    __u32 str_len   = strlen(str);
    __u32 half_len  = str_len/2;

    char tmp;
    __u32 j = str_len-1; 
    for ( __u32 i = 0; i < half_len; i++ ){
        tmp = str[i];
        str[i] = str[j];
        str[j--] = tmp;
    }

    return;
}