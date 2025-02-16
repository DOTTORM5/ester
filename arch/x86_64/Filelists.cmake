set(ESTER_ARCH_x86_64_ASM
    ${ESTER_DIR}/arch/x86_64/boot.asm
    ${ESTER_DIR}/arch/x86_64/long_mode_start.asm
    ${ESTER_DIR}/arch/x86_64/multiboot_header.asm
    ${ESTER_DIR}/arch/x86_64/idt.asm
)

set_source_files_properties(${ESTER_ARCH_x86_64_ASM} PROPERTIES LANGUAGE ASM_NASM)

target_sources(ester.bin PRIVATE ${ESTER_ARCH_x86_64_ASM})