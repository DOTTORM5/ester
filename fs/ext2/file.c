
#include "ext2.h"
#include "mem.h"
#include "printk.h"
#include "string.h"

/* For now only one ext2 file at a time can be opened */
static ext2_file_t ext2_file;

/**
 * Ext2 get the file descriptor. 
 * @param void
 * @return ext2_file_t * a pointer to the file descriptor
 */
ext2_file_t * ext2_file_get (void)
{
    return &ext2_file;
} 

/**
 * Ext2 file open. Open a file.  
 * @param const char * pathname the name (absolute path) of the file to be opened.
 * @param const char * mode the mode with which open the file.
 * @return ext2_file_t * a pointer to the file struct of the opened file
 */
ext2_file_t * ext2_fopen( const char * pathname, const char * mode ) 
{

    /* Get the file structure */
    ext2_file_t * f = ext2_file_get(); 

    if ( strlen(pathname) > MAX_PATH_LEN ) {   /* TODO Also the depth should be checked */
        printk("The pathname is too long, can't open\n");
        return f; 
    }
    
    if ( f->open ) {
        printk("A file is already opened, please close it before opening another.\n");
        return f; /* Return the already opened file */
    }

    /* Find the file inode number */ 
    uint32_t inode_num = ext2_inode_find(pathname);

    /* Extract the inode from the disk */
    ext2_inode_t * inode = ext2_extract_inode(inode_num);

    /* Check if the inode points to a real file */ 
    if ( ! (inode->i_type_permission & 0x8000) ) {    /* The file does not exist        */
        if ( mode[0] != 'w' ) {                       /* The mode is not write -> can't open */
            printk("The file does not exist!\n");
            return f; /* should be NULL */  
        } else {
            f->exist = 0;   /* The file can be open because it will be created later, but of course it does not exist */
        }
    } else {
        f->exist = 1;
    }

    /* Set the file struct properly */
    f->open = 1; 
    f->pos = 0;
    f->dirty = 0; 
    f->inode_num = inode_num; 

    /* TODO - check the size of mode and sanitize it properly*/
    memcpy(f->mode, mode, sizeof(mode)); 
    /* Copy the inode into the file struct */
    memcpy(&f->inode, inode, sizeof(ext2_inode_t)); 
    /* Copy the pathname into the file struct */
    memcpy(f->pathname, pathname, strlen(pathname)); 
    /* Set the buffer to 0 */
    memset(&f->buff, 0x0, sizeof(f->buff)); 

    return f; 

}

/* TODO - for now not update the metadata on the disk this function */
uint32_t ext2_free_inode_find( uint32_t group )
{
    ext2_block_group_descriptor * bgd = ext2_get_bgd(group);

    for (uint32_t i = 0; i < INODES_PER_GROUP; i++) {
        if ((bgd->bg_inode_bitmap & (1 << i)) == 0) {
            
            /* Mark inode as used */ 
            bgd->bg_inode_bitmap |= (1 << i);        

            /* Update metadata (decrease free inodes count) */ 
            bgd->bg_free_inodes_count--;

            return i + 1; /* Inode numbers start from 1 */ 
        }
    }

    return 0; /* NO more free inode */

}

/**
 * Ext2 file create. Create a file on the disk, the parent directory of the file must exist. 
 * @param const char * pathname the path of the file to be created. 
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_file_create( const char * pathname ) 
{
    /* Check the pathname length */
    if ( strlen(pathname) > MAX_PATH_LEN ) {
        printk("The pathname is too long, can't open\n");
        return 1; 
    }

    char unpacked_path[MAX_PATH_DEPTH][EXT2_NAME_LEN+2]; 
    uint16_t path_depth = unpack_dir_path(pathname, unpacked_path);

    /* Check the path depth */
    if ( path_depth > MAX_PATH_DEPTH ) {
        printk("The path is too deep, can't open\n");
        return 1; 
    }

    /* Build the parent dir pathname */
    char parent_pathname [MAX_PATH_LEN];
    for (uint16_t i = 0; i<path_depth-1; i++ ){
        strncat(parent_pathname, unpacked_path[i], strlen(unpacked_path[i])); 
    }
    
    /* Find the inode of the parent directory */
    uint32_t parent_inode_num = ext2_inode_find(parent_pathname);
    
    /* Extract the inode of the parent directory */
    ext2_inode_t * inode = ext2_extract_inode(parent_inode_num);

    /* Check if the parent is really a directory */
    if ( ! (inode->i_type_permission & 0x4000) ) {
        printk ("The parent is not a directory\n");
        return 1;
    }

    /* Get the superblock */
    ext2_super_block * sb = ext2_get_sb(); 

    /* Compute the group of the parent dir inode */
    uint32_t parent_group_num = (parent_inode_num - 1) / sb->s_inodes_per_group;

    /* Get a free inode number */
    uint32_t inode_num = ext2_free_inode_find(parent_group_num);

}


/**
 * Ext2 file close. Close a file. 
 * @param ext2_file_t * f a pointer to the file to close. 
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_fclose( ext2_file_t * f ) 
{
    if ( !f->open ) {
        printk("The file is not open.\n");
        return 1;
    }    

    /* Check if the file exists on the disk */
    if ( !f->exist ) {   /* the file does not exist, needs to be created */

        ext2_file_create(f->pathname);

    }





    /* Check if the file is dirty */
    if ( f->dirty ) { /* The file is dirty, need to store the buffer on the disk */



    }

    /* Set the file struct properly */
    f->open = 0; 
    f->pos  = 0; 
    f->exist = 0; 
    f->inode_num = 0; 
    f->dirty = 0; 
    memset(&f->mode, 0x0, sizeof(f->mode)); 
    memset(&f->inode, 0x0, sizeof(ext2_inode_t)); 
    memset(&f->pathname, 0x0, sizeof(f->pathname)); 
    memset(&f->buff, 0x0, sizeof(f->buff)); 
    return 0; 
}

/**
 * Ext2 file read. Read size bytes from a file. 
 * @param ext2_file_t * f a pointer to the file to read.
 * @param uint8_t * buff a pointer to the buffer where store the read data.
 * @param uint32_t size the number of bytes to read
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_fread( ext2_file_t * f, uint8_t * buff, uint32_t size ) 
{
    if ( !f->open ) {
        printk("The file is not open.\n");
        return 1;
    }    

    
    // ext2_read_file(); 


    return 0; 
}


