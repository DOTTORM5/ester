set(BLOCK_DEVICE
    ${ESTER_DIR}/drivers/block/block_device.c 
)

include_directories(
    ${ESTER_DIR}/drivers/block
)

target_sources(ester.bin PRIVATE ${BLOCK_DEVICE})