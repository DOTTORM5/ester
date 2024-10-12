set(VGA_FILES
    ${ESTER_DIR}/drivers/vga/vga.c 
)

include_directories(
    ${ESTER_DIR}/drivers/vga
)

target_sources(ester.bin PRIVATE ${VGA_FILES})