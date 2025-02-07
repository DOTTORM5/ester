#include "string.h"

/**
 * Strings compare. Compares two strings
 * @param const char *str1 a pointer to the first string to be compared
 * @param const char *str2 a pointer to the second string to be compared
 * @return uint8_t 0 if str1 == str2, 1 if str1 > str2, -1 if str1 < str2 
 */
uint8_t strcmp(const char *str1, const char *str2) 
{
    uint32_t i = 0; 
    while ( str1[i] && str2[i] && (str1[i] == str2[i])) i++; 

    if (str1[i] < str2[i]) { 
        return -1;
    } else if (str1[i] > str2[i]) {
        return 1;
    } else {
        return 0; 
    }

}

/* String length */
uint32_t strlen(const char *str)
{   
    uint32_t len = 0;
    while(str[len]){
        len++;
    }
    return len;
}

/* Sting copy - we need to introduce ERROR CODES*/
uint8_t strcpy(const char *str_src, char *str_dst)
{
    uint32_t src_len = strlen(str_src);
    uint32_t dst_len = strlen(str_dst);

    if (src_len != dst_len) return 1; 

    for ( uint32_t i = 0; i < dst_len; i++ ) {
        str_dst[i] = str_src[i];
    }

    return 0;
}

/* String reverse */
void strrev (char *str) 
{
    uint32_t str_len   = strlen(str);
    uint32_t half_len  = str_len/2;

    char tmp;
    uint32_t j = str_len-1; 
    for ( uint32_t i = 0; i < half_len; i++ ){
        tmp = str[i];
        str[i] = str[j];
        str[j--] = tmp;
    }

    return;
}