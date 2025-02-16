set(8259_PIC_FILES
    ${ESTER_DIR}/drivers/8259_pic/8259_pic.c 
)

include_directories(
    ${ESTER_DIR}/drivers/8259_pic
)

target_sources(ester.bin PRIVATE ${8259_PIC_FILES})