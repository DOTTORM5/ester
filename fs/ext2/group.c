#include "ext2.h"
#include "block_device.h"
#include "printk.h"
#include "mem.h"


#define MAX_GROUPS 8192  // For now we allocate an entire bgdt (block group descriptor table) in memory as static array, we need to optimize it... 

static struct ext2_block_group_descriptor bgdt[MAX_GROUPS];

/**
 * Ext2 block group descriptor table extractor. This function is invoked when mounting the fs
 * @param void for now use no param, simply use the static allocated struct of this file. Need to be optimized
 * @return uint8_t non-zero value for errors
 */
uint8_t ext2_extract_bgdt(void)
{
    struct block_device *dev = get_block_device(); // Get the global block device, very very very ugly bad and ridicolous, TODO - change this please
    
    // We assume for now that the block size is 4k (4096), of course this can change and the following code could not work
    // This is a dummy code just to have things working...
    char buffer[4096];
    dev->read_blocks(1, 1, buffer); 

    memcpy(&bgdt, buffer, 1024);

    printk("Read the BGDT\n"); 
    printk("Block[0].bg_inode_table: %d\n", bgdt[0].bg_inode_table); 

    return 0; 

}

/**
 * Ext2 get block group descriptor. 
 * @param uint32_t group_num the number of the block_group in the table
 * @return struct ext2_block_group_descriptor * a pointer to the group descriptor
 */
struct ext2_block_group_descriptor ext2_get_bgd(uint32_t group_num)
{
    return bgdt[group_num];
}   