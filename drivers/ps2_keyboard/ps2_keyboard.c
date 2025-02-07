/* THIS IS NOT ONLY THE KEYBOARD ... PORT 2 IS FOR THE MOUSE BUT IT IS NOT USED YET ... */

#include "ps2_keyboard.h"
#include "idt.h"
#include "printk.h"
#include "utils.h"
#include "8259_pic.h"

#define PS2_KEYBOARD_DATA_PORT           0x60
#define PS2_KEYBOARD_CTRL_PORT           0x64
#define PS2_KEYBOARD_CMD_DISABLE_FIRST   0xAD   // disable keyboard
#define PS2_KEYBOARD_CMD_ENABLE_FIRST    0xAE   // enable  keyboard 
#define PS2_KEYBOARD_CMD_DISABLE_SECOND  0xA7   // disable mouse
#define PS2_KEYBOARD_CMD_ENABLE_SECOND   0xA8   // enable  mouse

#define PS2_KEYBOARD_INT_VECTOR_ABSOLUTE 33

#define PS2_KEYBOARD_MAX_RETRIES         3

/* Scan codes 1 */
#define PS2_KEYBOARD_NULL_KEY     0
#define PS2_KEYBOARD_Q_PRESSED    0x10
#define PS2_KEYBOARD_Q_RELEASED   0x90
#define PS2_KEYBOARD_W_PRESSED    0x11
#define PS2_KEYBOARD_W_RELEASED   0x91
#define PS2_KEYBOARD_E_PRESSED    0x12
#define PS2_KEYBOARD_E_RELEASED   0x92
#define PS2_KEYBOARD_R_PRESSED    0x13
#define PS2_KEYBOARD_R_RELEASED   0x93
#define PS2_KEYBOARD_T_PRESSED    0x14
#define PS2_KEYBOARD_T_RELEASED   0x94
#define PS2_KEYBOARD_Z_PRESSED    0x15
#define PS2_KEYBOARD_Z_RELEASED   0x95
#define PS2_KEYBOARD_U_PRESSED    0x16
#define PS2_KEYBOARD_U_RELEASED   0x96
#define PS2_KEYBOARD_I_PRESSED    0x17
#define PS2_KEYBOARD_I_RELEASED   0x97
#define PS2_KEYBOARD_O_PRESSED    0x18
#define PS2_KEYBOARD_O_RELEASED   0x98
#define PS2_KEYBOARD_P_PRESSED    0x19
#define PS2_KEYBOARD_P_RELEASED   0x99
#define PS2_KEYBOARD_A_PRESSED    0x1E
#define PS2_KEYBOARD_A_RELEASED   0x9E
#define PS2_KEYBOARD_S_PRESSED    0x1F
#define PS2_KEYBOARD_S_RELEASED   0x9F
#define PS2_KEYBOARD_D_PRESSED    0x20
#define PS2_KEYBOARD_D_RELEASED   0xA0
#define PS2_KEYBOARD_F_PRESSED    0x21
#define PS2_KEYBOARD_F_RELEASED   0xA1
#define PS2_KEYBOARD_G_PRESSED    0x22
#define PS2_KEYBOARD_G_RELEASED   0xA2
#define PS2_KEYBOARD_H_PRESSED    0x23
#define PS2_KEYBOARD_H_RELEASED   0xA3
#define PS2_KEYBOARD_J_PRESSED    0x24
#define PS2_KEYBOARD_J_RELEASED   0xA4
#define PS2_KEYBOARD_K_PRESSED    0x25
#define PS2_KEYBOARD_K_RELEASED   0xA5
#define PS2_KEYBOARD_L_PRESSED    0x26
#define PS2_KEYBOARD_L_RELEASED   0xA6
#define PS2_KEYBOARD_Y_PRESSED    0x2C
#define PS2_KEYBOARD_Y_RELEASED   0xAC
#define PS2_KEYBOARD_X_PRESSED    0x2D
#define PS2_KEYBOARD_X_RELEASED   0xAD
#define PS2_KEYBOARD_C_PRESSED    0x2E
#define PS2_KEYBOARD_C_RELEASED   0xAE
#define PS2_KEYBOARD_V_PRESSED    0x2F
#define PS2_KEYBOARD_V_RELEASED   0xAF
#define PS2_KEYBOARD_B_PRESSED    0x30
#define PS2_KEYBOARD_B_RELEASED   0xB0
#define PS2_KEYBOARD_N_PRESSED    0x31
#define PS2_KEYBOARD_N_RELEASED   0xB1
#define PS2_KEYBOARD_M_PRESSED    0x32
#define PS2_KEYBOARD_M_RELEASED   0xB2

#define PS2_KEYBOARD_ZERO_PRESSED  0x29
#define PS2_KEYBOARD_ONE_PRESSED   0x2
#define PS2_KEYBOARD_NINE_PRESSED  0xA

#define PS2_KEYBOARD_POINT_PRESSED   0x34
#define PS2_KEYBOARD_POINT_RELEASED  0xB4

#define PS2_KEYBOARD_SLASH_RELEASED  0xB5

#define PS2_KEYBOARD_BACKSPACE_PRESSED  0xE
#define PS2_KEYBOARD_BACKSPACE_RELEASED  0x8E
#define PS2_KEYBOARD_SPACE_PRESSED  0x39
#define PS2_KEYBOARD_SPACE_RELEASED  0xB9
#define PS2_KEYBOARD_ENTER_PRESSED  0x1C
#define PS2_KEYBOARD_ENTER_RELEASED  0x9C


/* To be analyzed better */
static uint8_t __ps2_keyboard_wait_ack(void) 
{
	uint32_t timeout = 100000;
	while (--timeout) {
		if (!(inb(0x64) & (1 << 1))) return 0;
	}
	return 1;
}

/* Very dangerous - need a timeout */
static void __ps2_keyboard_clear_input_data_buffer(void)
{
    while ((inb(0x64) & 1)) inb(0x60);
}

static uint8_t __ps2_keyboard_read_scan_code(void) 
{
    return inb(0x60);
} 

static uint8_t __ps2_keyboard_send_cmd(uint8_t cmd) 
{
    outb(0x64, cmd);
    for ( uint8_t i = 0; i < PS2_KEYBOARD_MAX_RETRIES; i++ ) {
        if ( __ps2_keyboard_wait_ack() == 0 ) return 0;
        outb(0x64, cmd);
    }
    return 1; 
} 

static uint8_t __ps2_keyboard_disable_first(void)
{
    return __ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_DISABLE_FIRST);
}

static uint8_t __ps2_keyboard_enable_first(void)
{
    return __ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_ENABLE_FIRST);
}

static uint8_t __ps2_keyboard_disable_second(void)
{
    return __ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_DISABLE_SECOND);
}

static uint8_t __ps2_keyboard_enable_second(void)
{
    return __ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_ENABLE_SECOND);
}

static char* _qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";
static uint8_t __ps2_keyboard_to_ascii(uint8_t scan_code)
{
    if(scan_code == 0x1C) return '\n';
	if(scan_code == 0x39) return ' ';
	if(scan_code == 0xE) return '\r';
	if(scan_code == PS2_KEYBOARD_POINT_RELEASED) return '.';
	if(scan_code == PS2_KEYBOARD_SLASH_RELEASED) return '/';
	if(scan_code == PS2_KEYBOARD_ZERO_PRESSED) return '0';
	if(scan_code >= PS2_KEYBOARD_ONE_PRESSED && scan_code <= PS2_KEYBOARD_NINE_PRESSED)
		return _num[scan_code - PS2_KEYBOARD_ONE_PRESSED];
	if(scan_code >= 0x10 && scan_code <= 0x1C)
	{
		return _qwertzuiop[scan_code - 0x10];
	} else if(scan_code >= 0x1E && scan_code <= 0x26)
	{
		return _asdfghjkl[scan_code - 0x1E];
	} else if(scan_code >= 0x2C && scan_code <= 0x32)
	{
		return _yxcvbnm[scan_code - 0x2C];
	}
	return 0;
} 

/* SUPER UGLY - need to manage the interrupt flow... */
/* static ? */ void ps2_keyboard_interrupt_handler(void)
{
    __asm__ volatile ("cli");
    
    uint8_t scan_code = __ps2_keyboard_read_scan_code();
    uint8_t c = __ps2_keyboard_to_ascii(scan_code); 

    if (c != 0) printk("%c", c);

    pic_send_eoi(1);
    __asm__ volatile ("sti");
    __asm__ volatile ("leave");
    __asm__ volatile ("iretq");
}


uint8_t ps2_keyboard_init()
{
    if (__ps2_keyboard_disable_first()){
        // Error
        return 1;
    }

    if (__ps2_keyboard_disable_second()) {
        // Error
        return 1;
    }
    __ps2_keyboard_clear_input_data_buffer();
    
    if (__ps2_keyboard_enable_first()){
        // Error
        return 1;
    }

    if (__ps2_keyboard_enable_second()) {
        // Error
        return 1;
    }

    register_interrupt_handler(PS2_KEYBOARD_INT_VECTOR_ABSOLUTE, (uint64_t) ps2_keyboard_interrupt_handler);

    return 0;
}