#ifndef KERNEL_H
#define KERNEL_H

#define GDT_OFFSET_KERNEL_CODE 0x08

void kernel_entry( uint32_t m2_info_address );

#endif // KERNEL_H