#include "ext2.h"
#include "block_device.h"
#include "mem.h"
#include "printk.h"
#include "string.h"

static ext2_inode_t __current_inode;

/**
 * Ext2 get the current inode. 
 * @param void
 * @return ext2_inode_t * a pointer to the current inode
 */
ext2_inode_t * ext2_get_current_inode(void)
{
    return &__current_inode;
}   

/**
 * Ext2 Inode number find. Find the inode number of a given pathname, this works with both dir and file
 * @param const char * pathname the path of the file/dir of which to find the inode 
 * @return uint32_t the inode number
 */
uint32_t ext2_inode_find ( const char * pathname ) 
{
    char unpacked_path [MAX_PATH_DEPTH][EXT2_NAME_LEN+2];  /* +2 because one is for the null terminator and one is for the / char */
    uint16_t path_depth = unpack_dir_path(pathname, unpacked_path);

    /* Find the new inode number */
    uint32_t inode_num = 2; /* start from the root dir */
    uint32_t old_inode_num = inode_num; /* To keep track of the old inode in case no name is matched */
    ext2_dir_entry_fixed_name_t dir_entries[MAX_SUBDIRS]; 
    uint16_t entries_count = 0;    

    if (strcmp(pathname, "/") == 0) { /* Basic case when the pathname is the root */
        return inode_num;         
    } 
    for (uint16_t i = 1; i < path_depth; i++){
        old_inode_num = inode_num; 

        entries_count = ext2_list_directory(inode_num, dir_entries); 
        for (uint16_t j = 0; j < entries_count; j++){
            if ( strcmp(unpacked_path[i], dir_entries[j].dir_name) == 0 ) {
                inode_num = dir_entries[j].dir_inode;
                break;
            }
        }
        /* The inode is the same, not found a valid dir o file */
        if ( old_inode_num == inode_num ) {
            break; 
        }
    }
    return inode_num; 
}
 

/**
 * Ext2 inode extractor. Reads the inode indexed by inode_num on demand 
 * @param uint32_t inode_num the number of the inode
 * @return ext2_inode_t * a pointer to the extracted (now current) inode
 */
ext2_inode_t * ext2_extract_inode(uint32_t inode_num) 
{

    ext2_inode_t *inode = ext2_get_current_inode();
 
    /* Get the inode size */
    uint16_t inode_size = ext2_get_inode_size();

    /* Compute the gorup and the index of the inode inside the inode table */
    uint32_t group = (inode_num - 1) / ext2_get_inodes_per_group();
    uint32_t index = (inode_num - 1) % ext2_get_inodes_per_group();
    
    /* Locate the inode table block */ 
    uint32_t inode_table_block = ext2_get_bgd(group)->bg_inode_table;

    /* Compute the block size */
    uint32_t block_size = ext2_get_block_size();

    /* Compute the block and offset within the table */
    uint32_t block = inode_table_block + (index * inode_size) / block_size;
    uint32_t offset = (index * inode_size) % (block_size);

    /* Read the block containing the inode */
    uint8_t buffer[block_size];
    ext2_read_block(block, buffer);

    /* Extract inode from buffer */ 
    memcpy(inode, buffer+offset, sizeof(ext2_inode_t));

    return inode;
}

/**
 * Ext2 inode write. Save an inode on the disk (commit) WARNING this is a state-modifier function.  
 * @param uint32_t inode_num the inode number.
 * @param ext2_inode_t * inode the inode to be saved.
 * @return void.
 */
void ext2_inode_write(uint32_t inode_num, ext2_inode_t * inode)
{ 
    /* Get the inode size */
    uint16_t inode_size = ext2_get_inode_size();  

    /* Compute the gorup and the index of the inode inside the inode table */
    uint32_t group = (inode_num - 1) / ext2_get_inodes_per_group();
    uint32_t index = (inode_num - 1) % ext2_get_inodes_per_group();
    
    /* Locate the inode table block */ 
    uint32_t inode_table_block = ext2_get_bgd(group)->bg_inode_table;

    /* Compute the block size */
    uint32_t block_size = ext2_get_block_size();

    /* Compute the block and offset within the table */
    uint32_t block = inode_table_block + (index * inode_size) / block_size;
    uint32_t offset = (index * inode_size) % (block_size);

    /* Read the block containing the inode */
    uint8_t buffer[block_size];
    ext2_read_block(block, buffer);

    /* Copy the inode to write into the buffer */
    memcpy(buffer+offset, inode, sizeof(ext2_inode_t)); 

    /* Write the entire block back to the disk */
    ext2_write_block(block, buffer); 
    return;
}