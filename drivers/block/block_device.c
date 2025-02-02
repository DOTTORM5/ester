#include "block_device.h"
#include "ahci.h"
#include "printk.h"

static struct block_device *global_blk_dev;

// Function to register a block device (AHCI or others)
void register_block_device(struct block_device *dev) {
    global_blk_dev = dev;
}

// Function to retrieve the registered block device
struct block_device *get_block_device() {
    return global_blk_dev;
}

// AHCI-backed block device implementation
 
#define SECTOR_SIZE 512           // AHCI typically uses 512-byte sectors
#define LOGICAL_BLOCK_SIZE 1024   // EXT2 default block size

static uint8_t ahci_read_wrapper(uint64_t lbn, uint32_t count, void *buffer) {
    uint64_t lba = (lbn * LOGICAL_BLOCK_SIZE) / SECTOR_SIZE; // Translate LBN to LBA
    uint32_t sector_count = (count * LOGICAL_BLOCK_SIZE) / SECTOR_SIZE;
    return ahci_read(0, lba, sector_count, buffer);  // Assume AHCI 0 
}

static uint8_t ahci_write_wrapper(uint64_t lbn, uint32_t count, const void *buffer) {
    uint64_t lba = (lbn * LOGICAL_BLOCK_SIZE) / SECTOR_SIZE;
    uint32_t sector_count = (count * LOGICAL_BLOCK_SIZE) / SECTOR_SIZE;
    return ahci_write(0, lba, sector_count, buffer);
}

// Create an AHCI-based block device
static struct block_device ahci_device = {
    .logical_block_size = LOGICAL_BLOCK_SIZE,
    .physical_sector_size = SECTOR_SIZE,
    .read_blocks = ahci_read_wrapper,
    .write_blocks = ahci_write_wrapper
};

// Initialization function
void init_block_device() {
    register_block_device(&ahci_device);
}
