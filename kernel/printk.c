#include "vga.h"
#include "printk.h"
#include "string.h"


// Virtual print - call the right driver print function for now vga only
static void v_print_c(char c)
{
    return vga_print_char(c, BLACK, RED);
}

void printk(const char *str, ...)
{
    /* Start arg list */
    va_list arg_lst;
    va_start(arg_lst, str);

    /* Get the string length - security check */
    __u32 str_len = strlen(str);
    
    /* Placeholder token */
    char token = 'a';

    __u32 i = 0;
    while ( i < str_len ) {
        
        if ( str[i] == '%' ) {
            token = str[i+1];
        }
        else {
            token = 'a';
        }

        /* Check token - in another function */
        if ( token == 'd' ) {
            int arg = va_arg(arg_lst, int);
            
            /* Implement itoa */
            char hex_arg = (char) (arg + 48);

            v_print_c(hex_arg); 
            i++;
        } 
        i++;
    } 

    return;

}