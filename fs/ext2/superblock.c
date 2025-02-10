
#include "block_device.h"
#include "ext2.h"
#include "mem.h"
#include "printk.h"

static ext2_super_block sb; 

/**
 * Ext2 superblock extractor. This function is invoked when mounting the fs
 * @param void for now use no param, simply use the static allocated struct of this file. Need to be optimized
 * @param ext2_super_block * sb the superblock struct
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_extract_sb (void/*ext2_super_block * sb*/) 
{
    ext2_super_block *sb = ext2_get_sb();
    struct block_device *dev = get_block_device();
    if (!dev) {
        printk("No block device registered!\n");
        return -1;
    }

    uint8_t buffer[4096];  /* Superblock size is 1024 bytes, but i read the entire logic block that is 4096 bytes */ 

    /* Read the superblock from logical block 0 since each block is 4096 now */ 
    if (dev->read_blocks(0, 1, buffer) != 0) {
        printk("Failed to read superblock\n");
        return -1;
    }

    memcpy(sb, buffer+1024, sizeof(ext2_super_block));
    
    if (sb->s_magic != 0xEF53) {
        printk("Invalid EXT2 magic number: %s\n", sb->s_magic);
        return -1;
    }

    printk("EXT2 Detected! Blocks: %d, Inodes: %d, Block Size: %d\n", sb->s_blocks_count, sb->s_inodes_count, 1024 << sb->s_log_block_size);
    return 0;
}

/**
 * Ext2 get the superblock. 
 * @param void
 * @return ext2_super_block * a pointer to the superblock
 */
ext2_super_block * ext2_get_sb(void)
{
    return &sb;
}   