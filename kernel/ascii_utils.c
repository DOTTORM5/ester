#include "ascii_utils.h"
#include "types.h"
#include "string.h"
/* Int to ASCII */
/* Need to introduce also the third argument base, for now is dec only */
void itoa( int value, char * str /*, int base*/ )
{
    int residual = value;
    __u32 i = 0;
    do {
        str[i] = ( char ) ( (residual%10 + ASCII_NUM_BASE) );
        i++; 
        residual /= 10;
    } while (residual > 0);
    strrev(str);
    return;
}