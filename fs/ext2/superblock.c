
#include "block_device.h"
#include "ext2.h"
#include "mem.h"
#include "printk.h"

static ext2_super_block_t __sb; 

/**
 * Ext2 get the superblock. 
 * @param void
 * @return ext2_super_block_t * a pointer to the superblock
 */
static ext2_super_block_t * ext2_get_sb(void)
{
    return &__sb;
}   

uint32_t ext2_get_block_size(void)
{
    return 1024 << ext2_get_sb()->s_log_block_size;
}

uint32_t ext2_get_inodes_per_group(void)
{
    return ext2_get_sb()->s_inodes_per_group;
}

uint32_t ext2_get_inode_size(void)
{
    return ext2_get_sb()->s_inode_size;
}

/**
 * Ext2 superblock extractor. This function is invoked when mounting the fs
 * @param void for now use no param, simply use the static allocated struct of this file. Need to be optimized
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_extract_sb (void/*ext2_super_block_t * sb*/) 
{
    ext2_super_block_t *sb = ext2_get_sb();

    uint8_t buffer[4096];  /* TODO Superblock size is 1024 bytes, but i read the entire logic block that is 4096 bytes */ 

    /* Read the superblock from logical block 0 since each block is 4096 now */ 
    if (ext2_read_block(0, buffer) != 0) {
        printk("Failed to read superblock\n");
        return -1;
    }

    memcpy(sb, buffer+1024, sizeof(ext2_super_block_t));
    
    if (sb->s_magic != 0xEF53) {
        printk("Invalid EXT2 magic number: %s\n", sb->s_magic);
        return -1;
    }

    // printk("EXT2 Detected! Blocks: %d, Inodes: %d, Block Size: %d\n", sb->s_blocks_count, sb->s_inodes_count, 1024 << sb->s_log_block_size);
    return 0;
}

/**
 * Ext2 superblock write. This function save the superblock back on the disk
 * @param void
 * @return uint8_t non-zero value for errors
 */
static uint8_t ext2_write_sb (void) 
{
    ext2_super_block_t *sb = ext2_get_sb();
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t buffer[block_size]; 

    /* Read the superblock from logical block 0 since each block is 4096 now */ 
    if (ext2_read_block(0, buffer) != 0) {
        printk("Failed to read superblock\n");
        return 1;
    }

    /* Copy the superblock in the right position of the block buffer */
    memcpy(buffer+1024, sb, sizeof(ext2_super_block_t));
    ext2_write_block(0, buffer); 
    return 0;
}

/**
 * Ext2 inode alloc. Allocate a free inode inside group WARNING this is a state-modifier function.  
 * @param uint32_t group the group where to find the inode.
 * @return uint32_t the inode number allocated.
 */
uint32_t ext2_free_inode_alloc( uint32_t group )
{
    ext2_block_group_descriptor_t * bgd = ext2_get_bgd(group);
    ext2_super_block_t * sb = ext2_get_sb(); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Get the inode bitmap from the disk */
    uint8_t inode_bitmap [block_size];
    ext2_read_block(bgd->bg_inode_bitmap, inode_bitmap); 

    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
        if ( (inode_bitmap[i/8] & (1 << (i % 8 )) ) == 0) {
            
            /* Mark inode as used */ 
            inode_bitmap[i/8] |= (1 << (i % 8));      
            /* Save the new bitmap on the disk */  
            ext2_write_block(bgd->bg_inode_bitmap, inode_bitmap);

            /* Update metadata into the bgdt (decrease free inodes count) */ 
            bgd->bg_free_inodes_count--;
            ext2_write_bgdt(); 

            /* Update metadata into the superblock (decrease free inodes count) */
            sb->s_free_inodes_count--;
            ext2_write_sb();

            return (group * sb->s_inodes_per_group) + i + 1; /* Inode numbers start from 1 */ 
        }
    }
    return 0; /* NO more free inode */
}

 /**
  * Ext2 block alloc. Allocate a free block inside group WARNING this is a state-modifier function.  
  * @param uint32_t group the group where to find the block.
  * @return uint32_t the block number allocated.
  */
uint32_t ext2_free_block_alloc( uint32_t group )
{
    ext2_block_group_descriptor_t * bgd = ext2_get_bgd(group);
    ext2_super_block_t * sb = ext2_get_sb(); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Get the block bitmap from the disk */
    uint8_t block_bitmap [block_size];
    ext2_read_block(bgd->bg_block_bitmap, block_bitmap); 
    
    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {

        if ( (block_bitmap[i/8] & (1 << (i % 8 )) ) == 0) {
            
            /* Mark block as used */ 
            block_bitmap[i/8] |= (1 << (i % 8));      
            /* Save the new bitmap on the disk */  
            ext2_write_block(bgd->bg_block_bitmap, block_bitmap);

            /* Update metadata into the bgdt (decrease free blocks count) */ 
            bgd->bg_free_blocks_count--;
            ext2_write_bgdt(); 

            /* Update metadata into the superblock (decrease free blocks count) */
            sb->s_free_blocks_count--;
            ext2_write_sb();

            return (group * sb->s_blocks_per_group) + i; 
        }
    }
    return 0; /* NO more free block */
}
 