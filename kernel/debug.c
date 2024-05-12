#include "string.h"
#include "vga.h"
#include "debug.h"


/* NEED TO INTRODUCE TIMESTAMP!!! */
/* NEED TO INTRODUCE STRINGS IN GENERAL AND FORMATTED STRINGS */

void DEBUG_ERROR(char *msg)
{
    if (strlen(msg) > DEBUG_MSG_SIZE){
        DEBUG_ERROR("The msg is too long");
        return;
    }
    
    vga_print("[ ERROR ] ", BLACK, RED); 
    vga_print(msg, BLACK, RED);
    vga_print("\n", BLACK, RED); 
    return;
    
}

void DEBUG_WARNING(char *msg)
{
    if (strlen(msg) > DEBUG_MSG_SIZE){
        DEBUG_ERROR("The msg is too long");
        return;
    }
    vga_print("[ WARNING ] ", BLACK, YELLOW); 
    vga_print(msg, BLACK, YELLOW);
    vga_print("\n", BLACK, YELLOW); 
    return;
}

void DEBUG_INFO(char *msg)
{
    if (strlen(msg) > DEBUG_MSG_SIZE){
        DEBUG_ERROR("The msg is too long");
        return;
    }
    vga_print("[ INFO ] ", BLACK, WHITE); 
    vga_print(msg, BLACK, WHITE);
    vga_print("\n", BLACK, WHITE); 
    return;
}
