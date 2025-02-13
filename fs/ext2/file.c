
/* For now only one ext2 file at a time can be opened */
static ext2_file_t ext2_file;


ext2_file_t * ext2_file_get (void)
{
    return &ext2_file;
} 

/**
 * Ext2 file open. Open a file, for now no buffering!.  
 * @param const char * pathname the name (absolute path) of the file to be opened.
 * @param const char * mode the mode with which open the file.
 * @return ext2_file_t * a pointer to the file struct of the opened file
 */
ext2_file_t * ext2_fopen( const char * pathname, const char * mode ) 
{
    
    /* Get the file structure */
    ext2_file_t * f = ext2_file_get(); 

    if ( f->open ) {
        printk("A file is already opened, please close it before opening another.\n");
        return NULL;
    }

    /* Find the file inode number */ 
    uint32_t inode_num = ext2_inode_find(pathname);

    /* Extract the inode from the disk */
    ext2_inode_t * inode = ext2_extract_inode(inode_num);

    /* Check if the inode points to a real file */
    if ( ! (inode->i_type_permission & 0x8000) ) {
        printk("Not a file\n");
        return NULL;  
    } 

    /* Set the file struct properly */
    f->open = 1; 
    f->pos = 0;
    f->inode_num = inode_num; 
    memcpy(f->mode, mode, sizeof(f->mode)); 
    /* Copy the inode into the file struct */
    memcpy(f->inode, inode, sizeof(ext2_inode_t)); 

    return f; 

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

    /* Set the file struct properly */
    f->open = 0; 
    f->pos  = 0; 
    f->inode_num = 0; 
    memset(f->mode, 0x0, sizeof(f->mode)); 
    memset(f->inode, 0x0, sizeof(ext2_inode_t)); 
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

    
    ext2_read_file(); 


    return 0; 
}


