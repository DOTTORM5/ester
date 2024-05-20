#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_BASE_ADDRESS  0xb8000  /* VGA physical base address */
#define VGA_TXT_WIDTH     80       /* Default VGA text width  */
#define VGA_TXT_HEIGHT    25       /* Default VGA text height */

/* VGA colors */
#define BLACK    0x0
#define BLUE     0x1
#define GREEN    0x2
#define CYAN     0x3
#define RED      0x4
#define MAGENTA  0x5
#define BROWN    0x6
#define LGRAY    0x7
#define DGRAY    0x8
#define LBLUE    0x9
#define LGREEN   0xA
#define LCYAN    0xB
#define LRED     0xC
#define LMAGENTA 0xD
#define YELLOW   0xE
#define WHITE    0xF


typedef struct {
    volatile __u16 *txt_addr;      /* Text address pointer */
} __vga_device;

void vga_print(char *txt, unsigned char background, unsigned char foreground);

#endif