
#include "block_device.h"
#include "ext2.h"
#include "mem.h"
#include "printk.h"

/**
 * Ext2 superblock extractor. This function is invoked when mounting the fs
 * @param struct ext2_super_block * sb the superblock struct
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_extract_sb (struct ext2_super_block * sb) 
{
    
    struct block_device *dev = get_block_device();
    if (!dev) {
        printk("No block device registered!\n");
        return -1;
    }

    uint8_t buffer[1024];  /* Superblock size is 1024 bytes */ 

    /* Read the superblock from logical block 1 */ 
    if (dev->read_blocks(1, 1, buffer) != 0) {
        printk("Failed to read superblock\n");
        return -1;
    }

    memcpy(sb, buffer, sizeof(struct ext2_super_block));
    
    if (sb->s_magic != 0xEF53) {
        printk("Invalid EXT2 magic number: %s\n", sb->s_magic);
        return -1;
    }

    printk("EXT2 Detected! Blocks: %d, Inodes: %d, Block Size: %d\n", sb->s_blocks_count, sb->s_inodes_count, 1024 << sb->s_log_block_size);
    
    return 0;
}
