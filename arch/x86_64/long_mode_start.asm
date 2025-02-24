global long_mode_start
extern kernel_entry

section .text
bits 64
long_mode_start:

    ; reset all segment selector registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; call kernel entry passing to it the physical address provided by multiboot2, the kernel will find important informations at that address
    mov rdi, rbx
    call kernel_entry
    hlt