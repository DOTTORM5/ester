set(ESTER_KERNEL_SRC
    ${ESTER_DIR}/kernel/debug.c
    ${ESTER_DIR}/kernel/kernel.c
    ${ESTER_DIR}/kernel/string.c
    ${ESTER_DIR}/kernel/vga.c
    ${ESTER_DIR}/kernel/set_idt.c
    ${ESTER_DIR}/kernel/utils.c
    ${ESTER_DIR}/kernel/8259_pic.c
    ${ESTER_DIR}/kernel/multiboot2_infostruct_parser.c
    ${ESTER_DIR}/kernel/printk.c
)

set(ESTER_KERNEL_ASM
    ${ESTER_DIR}/kernel/boot.asm
    ${ESTER_DIR}/kernel/long_mode_start.asm
    ${ESTER_DIR}/kernel/multiboot_header.asm
    ${ESTER_DIR}/kernel/idt.asm
)

set(ESTER_KERNEL_LINKER
    ${ESTER_DIR}/kernel/linker.ld
)

set_source_files_properties(${ESTER_KERNEL_ASM} PROPERTIES LANGUAGE ASM_NASM)