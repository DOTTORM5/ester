set(EXT2
    ${ESTER_DIR}/fs/ext2/superblock.c 
)

include_directories(
    ${ESTER_DIR}/fs/ext2
)

target_sources(ester.bin PRIVATE ${EXT2})