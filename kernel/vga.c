#include "vga.h"

static __vga_device vga_device = { ( volatile __u16* ) VGA_BASE_ADDRESS };

static void vga_print_char(unsigned char c, unsigned char background, unsigned char foreground)
{
    /*  | BLINK (1) | BACKGROUND (3) | FOREGROUND (4) | CHAR (8) | */
    __u16 d = (__u16) ( (( ( (0x0f & background)  << 4 ) | ( 0x0f & foreground ) ) << 8 ) | ( __u16 ) c);
    *vga_device.txt_addr = d;
    return;
}

static void vga_clear_screen()
{
    vga_device.txt_addr = ( volatile __u16* ) VGA_BASE_ADDRESS;
    __u16 i = 0;
    while(i<(VGA_TXT_WIDTH*VGA_TXT_HEIGHT)) {
        vga_print_char(' ', BLACK, WHITE);
        vga_device.txt_addr++;
        i++;
    }
    vga_device.txt_addr = ( volatile __u16* ) VGA_BASE_ADDRESS;
    return;
}

static void vga_clear_line(__u16 line_i){
    __u16 i = 0;
    __u32 old_addr = (__u32) vga_device.txt_addr;
    vga_device.txt_addr = ( volatile __u16* ) VGA_BASE_ADDRESS + line_i*VGA_TXT_WIDTH;
    while(i < VGA_TXT_WIDTH) {
        vga_print_char(' ', BLACK, WHITE);
        vga_device.txt_addr++;
        i++;
    }
    vga_device.txt_addr = (volatile __u16 *) old_addr;
    return;
}

static void vga_scroll()
{
    __u16 i = 0;
    volatile __u16 * tmp; 
    vga_device.txt_addr = ( volatile __u16* ) VGA_BASE_ADDRESS;
    tmp = vga_device.txt_addr + VGA_TXT_WIDTH;
    while ( i < VGA_TXT_WIDTH*(VGA_TXT_HEIGHT-1)) {
        *vga_device.txt_addr = *tmp;
        vga_device.txt_addr++;
        tmp++;
        i++;
    }
    vga_clear_line(VGA_TXT_HEIGHT-1);  /* Clear the last line */
    return;
}


void vga_print(char *txt, unsigned char background, unsigned char foreground)
{
    __u16 i = 0;
    __u16 c_line = 0;     /* Current writing line */
    __u16 c_column = 0;   /* Current writing column (character) */

    while (txt[i]) {
        if ( ( __u32 ) vga_device.txt_addr ==  VGA_BASE_ADDRESS) {
            c_line = (__u16) (((__u32) (vga_device.txt_addr) - (__u32) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
        }
        else{
            c_line = (__u16) (((__u32) (vga_device.txt_addr-1) - (__u32) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
        }
        if (txt[i] == '\n' ) {
            vga_device.txt_addr = ( volatile __u16* ) VGA_BASE_ADDRESS + ((c_line+1) * VGA_TXT_WIDTH);
        }
        else {
            c_column = (__u16) (((__u32) (vga_device.txt_addr) - (__u32) VGA_BASE_ADDRESS) / (VGA_TXT_HEIGHT*2) );
            if ( (c_line >= VGA_TXT_HEIGHT-1) && (c_column == VGA_TXT_WIDTH) ) {
                vga_scroll();
            }
            vga_print_char(txt[i], background, foreground);
            vga_device.txt_addr++;
        }
        i++;
    }
    return;
} 