#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include "types.h"

struct block_device {
    uint32_t logical_block_size;               // Logical block size in bytes
    uint32_t physical_sector_size;             // Underlying sector size
    uint8_t (*read_blocks)(uint64_t lbn, uint32_t count, void *buffer);
    uint8_t (*write_blocks)(uint64_t lbn, uint32_t count, const void *buffer);
};

void register_block_device(struct block_device *dev);
struct block_device *get_block_device();

void init_block_device();

#endif
