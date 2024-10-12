/* VGA IS NOT THE RIGHT NAME TO GIVE TO THIS MODULE */
#include "vga.h"
#include "utils.h"
#include "types.h"

#define VGA_BASE_ADDRESS  0xb8000  /* VGA physical base address */
#define VGA_TXT_WIDTH     80       /* Default VGA text width  */
#define VGA_TXT_HEIGHT    25       /* Default VGA text height */

typedef struct {
    volatile uint16_t *txt_addr;      /* Text address pointer */
} __vga_device_t;

static __vga_device_t __vga_device = { ( volatile uint16_t* ) VGA_BASE_ADDRESS };

/* CURSOR */
static void __vga_update_cursor(uint16_t x, uint16_t y)
{
    uint16_t pos = y * VGA_TXT_WIDTH + x;
    outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF)); 
}

static void __vga_print_c(unsigned char c, unsigned char background, unsigned char foreground)
{
    /*  | BLINK (1) | BACKGROUND (3) | FOREGROUND (4) | CHAR (8) | */
    uint16_t d = (uint16_t) ( (( ( (0x0f & background)  << 4 ) | ( 0x0f & foreground ) ) << 8 ) | ( uint16_t ) c);
    *__vga_device.txt_addr = d;
    return;
}

static void __vga_clear_screen()
{
    __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS;
    uint16_t i = 0;
    while(i<(VGA_TXT_WIDTH*VGA_TXT_HEIGHT)) {
        __vga_print_c(' ', BLACK, WHITE);
        __vga_device.txt_addr++;
        i++;
    }
    __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS;
    return;
}

static void __vga_clear_line(uint16_t line_i){
    uint16_t i = 0;
    uint64_t old_addr = (uint64_t) __vga_device.txt_addr;
    __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS + line_i*VGA_TXT_WIDTH;
    while(i < VGA_TXT_WIDTH) {
        __vga_print_c(' ', BLACK, WHITE);
        __vga_device.txt_addr++;
        i++;
    }
    __vga_device.txt_addr = (volatile uint16_t *) old_addr;
    return;
}

static void __vga_scroll()
{
    uint16_t i = 0;
    volatile uint16_t * tmp; 
    __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS;
    tmp = __vga_device.txt_addr + VGA_TXT_WIDTH;
    while ( i < VGA_TXT_WIDTH*(VGA_TXT_HEIGHT-1)) {
        *__vga_device.txt_addr = *tmp;
        __vga_device.txt_addr++;
        tmp++;
        i++;
    }
    __vga_clear_line(VGA_TXT_HEIGHT-1);  /* Clear the last line */
    return;
}

/* Print a single char while keeping the screen formatting good */
void vga_print_char(unsigned char c, unsigned char background, unsigned char foreground)
{
    uint16_t c_line = 0;     /* Current writing line */
    uint16_t c_column = 0;   /* Current writing column (character) */
    uint16_t x = 0;
    uint16_t y = 0;

    if ( ( uint64_t ) __vga_device.txt_addr ==  VGA_BASE_ADDRESS) {
        c_line = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
    }
    else{
        c_line = (uint16_t) (((uint64_t) (__vga_device.txt_addr-1) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
    }
    
    if (c == '\n' ) {
        __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS + ((c_line+1) * VGA_TXT_WIDTH);
    }
    else {
        c_column = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_HEIGHT*2) );
        if ( (c_line >= VGA_TXT_HEIGHT-1) && (c_column == VGA_TXT_WIDTH) ) {
            __vga_scroll();
        }
        __vga_print_c(c, background, foreground);
        __vga_device.txt_addr++;
    }

    y = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
    x = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / 2 )  - y*VGA_TXT_WIDTH;
    __vga_update_cursor(x, y);
    return;
} 

/* Print an entire string while keeping the screen formatting good */
void vga_print(char * txt, unsigned char background, unsigned char foreground)
{
    uint16_t i = 0;
    uint16_t c_line = 0;     /* Current writing line */
    uint16_t c_column = 0;   /* Current writing column (character) */
    uint16_t x = 0;
    uint16_t y = 0;

    while (txt[i]) {
        if ( ( uint64_t ) __vga_device.txt_addr ==  VGA_BASE_ADDRESS) {
            c_line = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
        }
        else{
            c_line = (uint16_t) (((uint64_t) (__vga_device.txt_addr-1) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
        }
        if (txt[i] == '\n' ) {
            __vga_device.txt_addr = ( volatile uint16_t* ) VGA_BASE_ADDRESS + ((c_line+1) * VGA_TXT_WIDTH);
        }
        else {
            c_column = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_HEIGHT*2) );
            if ( (c_line >= VGA_TXT_HEIGHT-1) && (c_column == VGA_TXT_WIDTH) ) {
                __vga_scroll();
            }
            __vga_print_c(txt[i], background, foreground);
            __vga_device.txt_addr++;
        }
        i++;
    }

    y = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / (VGA_TXT_WIDTH*2) );
    x = (uint16_t) (((uint64_t) (__vga_device.txt_addr) - (uint64_t) VGA_BASE_ADDRESS) / 2 )  - y*VGA_TXT_WIDTH;
    __vga_update_cursor(x, y);
    return;
} 


