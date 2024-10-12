#ifndef VGA_H
#define VGA_H

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


void vga_print_char(unsigned char c, unsigned char background, unsigned char foreground);
void vga_print(char *txt, unsigned char background, unsigned char foreground);

#endif