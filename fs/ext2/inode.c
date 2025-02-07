#include "ext2.h"
#include "block_device.h"
#include "mem.h"
#include "printk.h"

static struct ext2_inode current_inode;

/**
 * Ext2 get the current inode. 
 * @param void
 * @return struct ext2_inode * a pointer to the current inode
 */
struct ext2_inode * ext2_get_current_inode(void)
{
    return &current_inode;
}   

/**
 * Ext2 inode extractor. Reads the inode indexed by inode_num on demand 
 * @param uint32_t inode_num the number of the inode
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_extract_inode(uint32_t inode_num) 
{

    struct ext2_inode *inode = ext2_get_current_inode();

    /* Get the block device (the storage) */
    struct block_device *dev = get_block_device();
    /* Get the superblock */
    struct ext2_super_block *sb = ext2_get_sb();
 
    /* Get the inode size */
    uint16_t inode_size = sb->s_inode_size;  

    /* Compute the gorup and the index of the inode inside the inode table */
    uint32_t group = (inode_num - 1) / sb->s_inodes_per_group;
    uint32_t index = (inode_num - 1) % sb->s_inodes_per_group;
    
    /* Locate the inode table block */ 
    uint32_t inode_table_block = ext2_get_bgd(group).bg_inode_table;

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Compute the block and offset within the table */
    uint32_t block = inode_table_block + (index * inode_size) / block_size;
    uint32_t offset = (index * inode_size) % (block_size);

    /* Read the block containing the inode */
    uint8_t buffer[block_size];
    dev->read_blocks(block, 1, buffer);

    /* Extract inode from buffer */ 
    memcpy(inode, buffer+offset, sizeof(struct ext2_inode));

    return 0;
}

/**
 * Ext2 list directory. Goes through directories printing files and directories, starting from dir_inode_num 
 * @param uint32_t dir_inode_num the number of the starting inode
 * @param struct ext2_dir_entry_fixed_name * dir_entries the struct to be updated with the entries of the requested directory
 * @return uint16_t the number of entries in the requested directory
 */
uint16_t ext2_list_directory(uint32_t dir_inode_num, struct ext2_dir_entry_fixed_name * dir_entries) 
{
    /* Get the block device (the storage) */
    struct block_device *dev = get_block_device();
    /* Get the superblock */
    struct ext2_super_block *sb = ext2_get_sb();

    struct ext2_inode *inode = ext2_get_current_inode();
    ext2_extract_inode(dir_inode_num); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;
    
    if (!(inode->i_type_permission & 0x4000)) { /* Check if it's a directory */
        printk("Not a directory!\n");
        return 0;
    }

    uint8_t buffer[block_size]; 
    
    /* Read each data block in the directory inode */
    uint16_t j = 0;
    for (int i = 0; i < 12 && inode->i_block[i]; i++) {
        dev->read_blocks(inode->i_block[i], 1, buffer);

        uint32_t offset = 0;
        while (offset < block_size) {

            if (j >= MAX_SUBDIRS) return j;

            struct ext2_dir_entry* entry = (struct ext2_dir_entry*)(buffer + offset);
            
            if (entry->dir_inode == 0) break; /* Empty entry */

            memcpy(dir_entries, entry, sizeof(struct ext2_dir_entry));             /* Copy the struct */
            memcpy(dir_entries->dir_name, entry->dir_name, entry->dir_name_len);   /* Copy the name   */

            offset += entry->dir_entry_size; /* Move to the next entry */
            dir_entries += 1;
            j++;
        }
    }

    return j;
}