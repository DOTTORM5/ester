#include "ps2_keyboard.h"
#include "idt.h"
#include "printk.h"
#include "utils.h"
#include "8259_pic.h"
#include "debug.h"

/* To be analyzed better */
static __u8 ps2_keyboard_wait_ack(void) 
{
	__u32 timeout = 100000;
	while (--timeout) {
		if (!(inb(0x64) & (1 << 1))) return 0;
	}
	return 1;
}

/* Very dangerous - need a timeout */
static void ps2_keyboard_clear_input_data_buffer(void)
{
    while ((inb(0x64) & 1)) inb(0x60);
}

/*static*/ __u8 ps2_keyboard_read_scan_code(void) 
{
    return inb(0x60);
} 

static __u8 ps2_keyboard_send_cmd(__u8 cmd) 
{
    outb(0x64, cmd);
    for ( __u8 i = 0; i < PS2_KEYBOARD_MAX_RETRIES; i++ ) {
        if ( ps2_keyboard_wait_ack() == 0 ) return 0;
        outb(0x64, cmd);
    }
    return 1; 
} 

static __u8 ps2_keyboard_disable_first(void)
{
    return ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_DISABLE_FIRST);
}

static __u8 ps2_keyboard_enable_first(void)
{
    return ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_ENABLE_FIRST);
}

static __u8 ps2_keyboard_disable_second(void)
{
    return ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_DISABLE_SECOND);
}

static __u8 ps2_keyboard_enable_second(void)
{
    return ps2_keyboard_send_cmd(PS2_KEYBOARD_CMD_ENABLE_SECOND);
}

static char* _qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char* _asdfghjkl = "asdfghjkl";
static char* _yxcvbnm = "yxcvbnm";
static char* _num = "123456789";
static __u8 ps2_keyboard_to_ascii(__u8 scan_code)
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

/* SUPER UGLY */
void ps2_keyboard_interrupt_handler(void)
{
    __asm__ volatile ("cli");
    
    __u8 scan_code = ps2_keyboard_read_scan_code();
    __u8 c = ps2_keyboard_to_ascii(scan_code); 

    if (c != 0) printk("%s", c);

    pic_send_eoi(1);
    __asm__ volatile ("sti");
    __asm__ volatile ("leave");
    __asm__ volatile ("iretq");
}


__u8 ps2_keyboard_init()
{
    if (ps2_keyboard_disable_first()){
        // Error
        return 1;
    }

    if (ps2_keyboard_disable_second()) {
        // Error
        return 1;
    }
    ps2_keyboard_clear_input_data_buffer();
    
    if (ps2_keyboard_enable_first()){
        // Error
        return 1;
    }

    if (ps2_keyboard_enable_second()) {
        // Error
        return 1;
    }

    register_interrupt_handler(33, (__u64) ps2_keyboard_interrupt_handler);

    return 0;
}