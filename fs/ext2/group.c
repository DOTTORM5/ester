#include "ext2.h"
#include "block_device.h"
#include "printk.h"
#include "mem.h"


#define MAX_GROUPS 128  // For now we allocate an entire bgdt (block group descriptor table) in memory as static array, we need to optimize it... 

static ext2_block_group_descriptor bgdt[MAX_GROUPS];

/**
 * Ext2 get block group descriptor. 
 * @param uint32_t group_num the number of the block_group in the table
 * @return ext2_block_group_descriptor * a pointer to the group descriptor
 */
ext2_block_group_descriptor * ext2_get_bgd(uint32_t group_num)
{
    return &bgdt[group_num];
}   

/**
 * Ext2 block group descriptor table extractor. This function is invoked when mounting the fs.
 * @param void for now use no param, simply use the static allocated struct of this file. Need to be optimized.
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_extract_bgdt(void)
{
    ext2_super_block * sb = ext2_get_sb(); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;
    char buffer[block_size];
    ext2_read_block(1, buffer); 
    memcpy(&bgdt, buffer, sizeof(bgdt));
    return 0; 

}

/**
 * Ext2 block group descriptor table writer. This function save the bgdt back to the disk
 * @param void for now use no param, simply use the static allocated struct of this file. Need to be optimized.
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_write_bgdt(void)
{
    ext2_super_block * sb = ext2_get_sb(); 
    
    /* Compute the block size */
    
    uint32_t block_size = 1024 << sb->s_log_block_size;
    char buffer[block_size];
    ext2_read_block(1, buffer); 

    memcpy(buffer, &bgdt, sizeof(bgdt));
    ext2_write_block(1, buffer);
    return 0; 

}

