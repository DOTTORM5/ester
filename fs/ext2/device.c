#include "block_device.h"
#include "ext2.h"


uint8_t ext2_read_block( uint32_t block_id, uint8_t * buff )
{
    /* Get the block device (the storage) */
    struct block_device *dev = get_block_device();
    return dev->read_blocks(block_id, 1, buff);
}

uint8_t ext2_write_block( uint32_t block_id, uint8_t * buff )
{
    /* Get the block device (the storage) */
    struct block_device *dev = get_block_device();
    return dev->write_blocks(block_id, 1, buff);
}