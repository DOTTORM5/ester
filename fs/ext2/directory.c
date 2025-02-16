#include "ext2.h"
#include "block_device.h"
#include "mem.h"
#include "string.h"
#include "printk.h"

static cwd_t __cwd; 

/**
 * Ext2 get the current working directory. 
 * @param void
 * @return cwd_t * a pointer to the current working directory structure
 */
cwd_t * ext2_get_cwd(void)
{
    return &__cwd;
}

/**
 * Ext2 init the current working directory to root /. 
 * @param void
 * @return void
 */
void ext2_init_cwd(void)
{
    cwd_t * cwd = ext2_get_cwd(); 
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
uint16_t unpack_dir_path(const char * dir_path, char unpacked_path[MAX_PATH_DEPTH][EXT2_NAME_LEN+2])
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
    cwd_t * cwd = ext2_get_cwd(); 

    char unpacked_path [MAX_PATH_DEPTH][EXT2_NAME_LEN+2];  /* +2 because one is for the null terminator and one is for the / char */

    uint16_t path_depth = unpack_dir_path(cwd_name, unpacked_path);

    /* Find the new inode number */
    uint32_t inode_dir_num = 2; /* start from the root dir */
    ext2_dir_entry_fixed_name_t dir_entries[MAX_SUBDIRS]; 
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
    return;
}


/**
 * Ext2 Add dir entry. Add an entry into a directory WARNING this is a state-modifier function. 
 * @param ext2_inode_t * dir_inode the inode of the directory. 
 * @param uint32_t new_entry_inode_num the inode number of the new entry.
 * @param const char * new_entry_name the name of the new entry.
 * @return uint8_t non-zero value in case of errors. 
 */
uint8_t ext2_add_dir_entry (ext2_inode_t * dir_inode, uint32_t new_entry_inode_num, const char * new_entry_name) 
{

    /* Check the size of new entry name */
    uint32_t block_size  = ext2_get_block_size();
    
    if (!(dir_inode->i_type_permission & 0x4000)) { /* Check if it's a directory */
        printk("Not a directory!\n");
        return 0;
    }

    uint8_t buffer[block_size]; 
    
    /* Read each data block in the directory inode */
    uint32_t j = 0;
    for (int i = 0; i < 12 && dir_inode->i_block[i]; i++) {
        ext2_read_block(dir_inode->i_block[i], buffer);

        uint32_t offset = 0;

        while (offset < block_size) {

            if (j >= MAX_SUBDIRS) return 1; /* The dir is full */

            ext2_dir_entry_t* current_entry = (ext2_dir_entry_t*)(buffer + offset);
            
            if (offset + current_entry->dir_entry_size >= block_size) { /* The last current_entry */

                uint32_t new_entry_size = sizeof(ext2_dir_entry_t) + strlen(new_entry_name);
                new_entry_size = (new_entry_size + 3) & ~3; /* Align to 4 bytes */

                /* Shrink the last current_entry to fit the new one */ 
                uint32_t old_rec_len = current_entry->dir_entry_size;
                current_entry->dir_entry_size = offset + new_entry_size;
                
                /* Create the new current_entry */
                ext2_dir_entry_t *new_entry = (ext2_dir_entry_t *) ((uint8_t *)current_entry + current_entry->dir_entry_size);
                /* Set the current_entry */
                new_entry->dir_inode = new_entry_inode_num;
                new_entry->dir_entry_size = old_rec_len - current_entry->dir_entry_size;
                new_entry->dir_name_len = strlen(new_entry_name);
                new_entry->dir_type = 0; 
                memcpy(&new_entry->dir_name, new_entry_name, strlen(new_entry_name));

                /* Write the buffer back to the disk */
                ext2_write_block(dir_inode->i_block[i], buffer);
                return 0;

            }
            j++;
            offset += current_entry->dir_entry_size; /* Move to the next current_entry */

        }
    }
    return 1;
}

/**
 * Ext2 list directory. Goes through directories printing files and directories, starting from dir_inode_num.
 * @param uint32_t dir_inode_num the number of the starting inode.
 * @param ext2_dir_entry_fixed_name_t * dir_entries the struct to be updated with the entries of the requested directory.
 * @return uint16_t the number of entries in the requested directory.
 */
uint16_t ext2_list_directory(uint32_t dir_inode_num, ext2_dir_entry_fixed_name_t * dir_entries) 
{
    ext2_inode_t *inode = ext2_get_current_inode();
    ext2_extract_inode(dir_inode_num); 

    /* Compute the block size */
    uint32_t block_size = ext2_get_block_size();
    
    if (!(inode->i_type_permission & 0x4000)) { /* Check if it's a directory */
        printk("Not a directory!\n");
        return 0;
    }

    uint8_t buffer[block_size]; 
    
    /* Read each data block in the directory inode */
    uint16_t j = 0;
    for (int i = 0; i < 12 && inode->i_block[i]; i++) {
        ext2_read_block(inode->i_block[i], buffer);

        uint32_t offset = 0;
        while (offset < block_size) {

            if (j >= MAX_SUBDIRS) return j;

            ext2_dir_entry_t* entry = (ext2_dir_entry_t*)(buffer + offset);
            
            if (entry->dir_inode == 0) break; /* Empty entry */

            memcpy(dir_entries, entry, sizeof(ext2_dir_entry_t));             /* Copy the struct */
            memcpy(dir_entries->dir_name, entry->dir_name, entry->dir_name_len);   /* Copy the name   */

            offset += entry->dir_entry_size; /* Move to the next entry */
            dir_entries += 1;
            j++;
        }
    }

    return j;
}

/* TODO - Create directory (?) */