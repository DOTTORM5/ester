set(EXT2
    ${ESTER_DIR}/fs/ext2/superblock.c
    ${ESTER_DIR}/fs/ext2/group.c 
    ${ESTER_DIR}/fs/ext2/inode.c 
    ${ESTER_DIR}/fs/ext2/directory.c 
)

include_directories(
    ${ESTER_DIR}/fs/ext2
)

target_sources(ester.bin PRIVATE ${EXT2})