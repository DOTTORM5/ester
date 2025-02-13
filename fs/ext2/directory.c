#include "ext2.h"
#include "block_device.h"
#include "mem.h"
#include "string.h"
#include "printk.h"

static __cwd cwd; 

/**
 * Ext2 get the current working directory. 
 * @param void
 * @return __cwd * a pointer to the current working directory structure
 */
__cwd * ext2_get_cwd(void)
{
    return &cwd;
}

/**
 * Ext2 init the current working directory to root /. 
 * @param void
 * @return void
 */
void ext2_init_cwd(void)
{
    __cwd * cwd = ext2_get_cwd(); 
    memcpy(cwd->cwd_name, "/", 1);
    cwd->cwd_inode_number = 2;  
    return; 
}

/**
 * Parse and unpack a directory path. 
 * @param const char * dir_path a pointer to the path string
 * @param char unpacked_path[][] the final unpacked path, each entry is a dir in the path 
 * @return uint16_t the number of directories in the path (the path depth)
 */
static uint16_t unpack_dir_path(const char * dir_path, char unpacked_path[MAX_PATH_DEPTH][EXT2_NAME_LEN+2])
{
    uint32_t i = 0; 
    uint32_t j = 0; 
    uint32_t k = 0;
    while (dir_path[i]) {  
        unpacked_path[j][k] = dir_path[i];
        if (dir_path[i] == '/') {
            if (j == 0){                      /* For the root dir we store the "/" as name of the dir */
                unpacked_path[j][k+1] = '\0'; /* Add the null terminator */ 
            } else {                          /* For non-root dir we eliminate the "/" at the end */
                unpacked_path[j][k] = '\0';   /* Add the null terminator */ 
            }
            j++;
            k = 0;
        } else {
            k++;
        }
        i++;
    }
    /* If you pass the path without the / as last char we need to still increase j */
    if (dir_path[i-1] != '/') {  
        unpacked_path[j][k+1] = '\0'; /* Add the null terminator */ 
        j++;
    }
    return j; 
}

/**
 * Ext2 change the current working directory to root /. 
 * @param char * cwd_name the new cwd name
 * @return void
 */
void ext2_change_cwd(const char * cwd_name)
{
    __cwd * cwd = ext2_get_cwd(); 

    char unpacked_path [MAX_PATH_DEPTH][EXT2_NAME_LEN+2];  /* +2 because one is for the null terminator and one is for the / char */

    uint16_t path_depth = unpack_dir_path(cwd_name, unpacked_path);

    /* Find the new inode number */
    uint32_t inode_dir_num = 2; /* start from the root dir */
    ext2_dir_entry_fixed_name dir_entries[MAX_SUBDIRS]; 
    uint16_t entries_count = 0;    

    if (strcmp(cwd_name, "/") == 0) { /* Basic case when the new cwd is the root */
        goto end;        
    } 

    for (uint16_t i = 1; i < path_depth; i++){
        entries_count = ext2_list_directory(inode_dir_num, dir_entries); 

        for (uint16_t j = 0; j < entries_count; j++){

            if ( strcmp(unpacked_path[i], dir_entries[j].dir_name) == 0 ) {
                inode_dir_num = dir_entries[j].dir_inode;
                break;
            }
        }
    }

end:
    cwd->cwd_inode_number = inode_dir_num;
    memcpy(cwd->cwd_name, cwd_name, strlen(cwd_name));

    /* Remove the / at the end of the path if the path is not root and there is a / */
    if ((strcmp(cwd_name, "/") != 0) && ( cwd_name[strlen(cwd_name)-1] == '/' )){
        cwd->cwd_name[strlen(cwd_name)-1] = '\0';        
    }

    // printk("CWD INODE: %d\n", cwd->cwd_inode_number);
    // printk("CWD NAME: %s\n", cwd->cwd_name);
    return;
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
    ext2_dir_entry_fixed_name dir_entries[MAX_SUBDIRS]; 
    uint16_t entries_count = 0;    

    if (strcmp(pathname, "/") == 0) { /* Basic case when the pathname is the root */
        return inode_num;         
    } 
    for (uint16_t i = 1; i < path_depth; i++){
        entries_count = ext2_list_directory(inode_num, dir_entries); 
        for (uint16_t j = 0; j < entries_count; j++){
            if ( strcmp(unpacked_path[i], dir_entries[j].dir_name) == 0 ) {
                inode_num = dir_entries[j].dir_inode;
                break;
            }
        }
    }
    return inode_num; 
}

/**
 * Ext2 read 4096 bytes from a file in memory. 
 * @param char * file_name the name of the file to be read
 * @param uint32_t offset the offset in the file where to read
 * @param uint8_t * ptr a pointer to the memory region to store the file - inout param
 * @return void
 */
void ext2_read_file( char * file_name, uint32_t offset, uint8_t * ptr ) 
{
    /* Get the current working directory */
    __cwd * cwd = ext2_get_cwd();   
    /* Get the block device (the storage) */
    struct block_device *dev = get_block_device();

    ext2_dir_entry_fixed_name dir_entries[MAX_SUBDIRS];
    
    /* the inode number of the file */
    uint32_t inode_file_num = 0;    

    /* Try to find the file in the current working directory, for now only this situation is supported */
    uint16_t entries_count = ext2_list_directory(cwd->cwd_inode_number, dir_entries);

    for (uint16_t j = 0; j < entries_count; j++){
        if ( strcmp(file_name, dir_entries[j].dir_name) == 0 ) {
            inode_file_num = dir_entries[j].dir_inode;
            break;
        }
    }

    if (inode_file_num == 0) {
        printk("Can't find the file in the current directory\n");
        return;
    }

    /* Extract the inode of the file */
    ext2_extract_inode(inode_file_num); 
        
    /* Get the current inode, updated above */
    ext2_inode_t *inode = ext2_get_current_inode();

    if ( ! (inode->i_type_permission & 0x8000) ) {
        printk("Not a file\n");
        return;  
    } 

    /* TODO compute the offset properly */
    dev->read_blocks(inode->i_block[offset], 1, ptr);
    
    return;
}