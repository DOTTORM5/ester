#include "vga.h"
#include "string.h"
#include "ascii_utils.h"
#include "printk.h"
#include <stdarg.h> 


// Print char - call the right driver print function - for now vga only
static void print_char (char c)
{
    vga_print_char(c, BLACK, LBLUE);
    return;
}

static void print_str(char *str)
{
    uint32_t i = 0; 
    while (str[i]) print_char(str[i++]); 
    return;
}

static void print_int (int value)
{
    char ascii_value[INT_MAX_DIGITS];  /* Need dynamic allocation... */
    itoa(value, ascii_value);
    uint32_t i = 0;
    while (ascii_value[i]) print_char(ascii_value[i++]);
    return;
} 

/* To be tested and optimized */
void printk (const char *str, ...)
{
    /* WARNING - NEED A CHECK ON THE VA ARGS LENGTH ETC*/
    /* Start arg list */
    va_list arg_lst;
    va_start(arg_lst, str);

    /* Invalid token - a */
    char token = 'a';
    uint32_t i = 0;
    
    while ( str[i] ) {
        if ( str[i] == '%' ) {
            token = str[i+1];
        }
        else {
            token = 'a';
        }

        /* Check token - to be handled better */
        if ( token == 'd' ) {
            print_int(va_arg(arg_lst, unsigned int));
            i++;
        }
        else if ( token == 's' ){
            print_str((char *)va_arg(arg_lst, uint64_t));
            i++;
        } 
        else if ( token == 'c' ) {
            print_char(va_arg(arg_lst, int));
            i++;
        }
        /* Not a valid token - to be handled better */
        else {
            print_char(str[i]); 
        }
        i++;
    }
    va_end(arg_lst);
    return;
}