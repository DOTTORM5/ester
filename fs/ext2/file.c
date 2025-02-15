
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


uint32_t ext2_free_inode_alloc( uint32_t group )
{
    ext2_block_group_descriptor * bgd = ext2_get_bgd(group);
    ext2_super_block * sb = ext2_get_sb(); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Get the inode bitmap from the disk */
    uint8_t inode_bitmap [block_size];
    ext2_read_block(bgd->bg_inode_bitmap, inode_bitmap); 

    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
        if ( (inode_bitmap[i/8] & (1 << (i % 8 )) ) == 0) {
            
            /* Mark inode as used */ 
            inode_bitmap[i/8] |= (1 << (i % 8));      
            /* Save the new bitmap on the disk */  
            ext2_write_block(bgd->bg_inode_bitmap, inode_bitmap);

            /* Update metadata into the bgdt (decrease free inodes count) */ 
            bgd->bg_free_inodes_count--;
            ext2_write_bgdt(); 

            /* Update metadata into the superblock (decrease free inodes count) */
            sb->s_free_inodes_count--;
            ext2_write_sb();

            return (group * sb->s_inodes_per_group) + i + 1; /* Inode numbers start from 1 */ 
        }
    }
    return 0; /* NO more free inode */
}


uint32_t ext2_free_block_alloc( uint32_t group )
{
    ext2_block_group_descriptor * bgd = ext2_get_bgd(group);
    ext2_super_block * sb = ext2_get_sb(); 

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Get the block bitmap from the disk */
    uint8_t block_bitmap [block_size];
    ext2_read_block(bgd->bg_block_bitmap, block_bitmap); 

    printk("HERE I :\n");
    
    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {

        


        if ( (block_bitmap[i/8] & (1 << (i % 8 )) ) == 0) {
            
            /* Mark block as used */ 
            block_bitmap[i/8] |= (1 << (i % 8));      
            /* Save the new bitmap on the disk */  
            ext2_write_block(bgd->bg_block_bitmap, block_bitmap);

            /* Update metadata into the bgdt (decrease free blocks count) */ 
            bgd->bg_free_blocks_count--;
            ext2_write_bgdt(); 

            /* Update metadata into the superblock (decrease free blocks count) */
            sb->s_free_blocks_count--;
            ext2_write_sb();


            return (group * sb->s_blocks_per_group) + i; 
        }
    }
    return 0; /* NO more free block */

}

uint8_t ext2_add_dir_entry (ext2_inode_t * dir_inode, uint32_t new_entry_inode_num, const char * new_entry_name) 
{
    /* Get the superblock */
    ext2_super_block *sb = ext2_get_sb();
    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;
    
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

            ext2_dir_entry* current_entry = (ext2_dir_entry*)(buffer + offset);
            
            if (offset + current_entry->dir_entry_size >= block_size) { /* The last current_entry */

                uint32_t new_entry_size = sizeof(ext2_dir_entry) + strlen(new_entry_name);
                new_entry_size = (new_entry_size + 3) & ~3; /* Align to 4 bytes */

                /* Shrink the last current_entry to fit the new one */ 
                uint32_t old_rec_len = current_entry->dir_entry_size;
                current_entry->dir_entry_size = offset + new_entry_size;
                
                /* Create the new current_entry */
                ext2_dir_entry *new_entry = (ext2_dir_entry *) ((uint8_t *)current_entry + current_entry->dir_entry_size);
                /* Set the current_entry */
                new_entry->dir_inode = new_entry_inode_num;
                new_entry->dir_entry_size = old_rec_len - current_entry->dir_entry_size;
                new_entry->dir_name_len = strlen(new_entry_name);
                new_entry->dir_type = 0; 
                memcpy(new_entry->dir_name, new_entry_name, strlen(new_entry_name));
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

void ext2_inode_write(uint32_t inode_num, ext2_inode_t * inode)
{
    /* Get the superblock */
    ext2_super_block *sb = ext2_get_sb();
 
    /* Get the inode size */
    uint16_t inode_size = sb->s_inode_size;  

    /* Compute the gorup and the index of the inode inside the inode table */
    uint32_t group = (inode_num - 1) / sb->s_inodes_per_group;
    uint32_t index = (inode_num - 1) % sb->s_inodes_per_group;
    
    /* Locate the inode table block */ 
    uint32_t inode_table_block = ext2_get_bgd(group)->bg_inode_table;

    /* Compute the block size */
    uint32_t block_size = 1024 << sb->s_log_block_size;

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


uint32_t ext2_sectors_to_blocks (uint32_t sectors)
{
    return (sectors * 512) / ( 1024 << ext2_get_sb()->s_log_block_size ); 
}

uint32_t ext2_blocks_to_sectors (uint32_t blocks)
{
    return (blocks * ( 1024 << ext2_get_sb()->s_log_block_size )) / 512; 
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
        printk("The pathname is too long, can't create\n");
        return 1; 
    }

    char unpacked_path[MAX_PATH_DEPTH][EXT2_NAME_LEN+2]; 
    uint16_t path_depth = unpack_dir_path(pathname, unpacked_path);

    /* Check the path depth */
    if ( path_depth > MAX_PATH_DEPTH ) {
        printk("The path is too deep, can't create\n");
        return 1; 
    }

    /* Build the parent dir pathname */
    char parent_pathname [MAX_PATH_LEN];
    for (uint16_t i = 0; i<path_depth-1; i++ ){
        strncat(parent_pathname, unpacked_path[i], strlen(unpacked_path[i])); 
    }

    /* TODO - Check if the file already exists */

    /* Find the inode of the parent directory */
    uint32_t parent_inode_num = ext2_inode_find(parent_pathname);
    
    /* Extract the inode of the parent directory */
    ext2_inode_t * parent_inode = ext2_extract_inode(parent_inode_num);

    /* Check if the parent is really a directory */
    if ( ! (parent_inode->i_type_permission & 0x4000) ) {
        printk ("The parent is not a directory\n");
        return 1;
    }


    /* Get the superblock */
    ext2_super_block * sb = ext2_get_sb(); 

    /* Compute the group of the parent dir inode */
    uint32_t parent_group_num = (parent_inode_num - 1) / sb->s_inodes_per_group;

    /* Get a free inode number */
    uint32_t inode_num = ext2_free_inode_alloc(parent_group_num);
    if ( inode_num != 0 ) {
        printk("No more free inodes in this group, can't create\n");
        return 1;
    } 
    
    /* Get a free block number */
    uint32_t block_num = ext2_free_block_alloc(parent_group_num);
    if ( block_num != 0 ) {
        printk("No more free blocks in this group, can't create\n");
        return 1;
    } 

    /* Create the new inode */
    ext2_inode_t new_inode; 
    new_inode.i_type_permission = 0x81A4; // Regular file
    new_inode.i_block[0] = block_num;
    new_inode.i_links_count = 1;
    new_inode.i_disk_sectors_count = ext2_blocks_to_sectors(1);
    new_inode.i_lower_size = 0; 
    ext2_inode_write(inode_num, &new_inode); 
    uint8_t err = ext2_add_dir_entry(parent_inode, inode_num, unpacked_path[path_depth-1]);
    if ( err != 0 ) {
        /* TODO - here the allocated inode and block should be ripristined as free, or they should never committed before this point... */
        printk("Error adding the dir entry\n");
        return 1;
    }

    /* SUCCESS THE FILE IS CREATED */
    return 0; 
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
    f->new_bytes_count = 0;
    f->inode_num = inode_num; 

    /* TODO - check the size of mode and sanitize it properly*/
    memcpy(f->mode, mode, 1); 
    /* Copy the inode into the file struct */
    memcpy(&f->inode, inode, sizeof(ext2_inode_t)); 
    /* Copy the pathname into the file struct */
    memcpy(f->pathname, pathname, strlen(pathname)); 
    /* Set the buffer to 0 */
    memset(&f->buff, 0x0, sizeof(f->buff)); 

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

    /* Check if the file exists on the disk */
    if ( !f->exist ) {   /* the file does not exist, needs to be created */
        if ( ext2_file_create(f->pathname) != 0 ) {
            printk("The file cannot be created for some errors\n");
            goto end;
        } 
        f->exist = 1;
    }

    ext2_super_block * sb = ext2_get_sb();
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* Check if the file is dirty */
    /* The actual buffer of the file is dirty, need to be saved on the disk before change the position */
    /* TODO put it into another function */
    if ( f->dirty  ) { 
        uint32_t block_id = f->pos/block_size; 
        uint32_t block_num = f->inode.i_block[block_id];  /* TODO For now we support only direct blocks */
        /* Save the current buffer on the disk */
        ext2_write_block(block_num, f->buff);
        if ( f->new_bytes_count > 0 ) {  /* There are some new bytes to store */
            f->inode.i_lower_size = f->pos+f->new_bytes_count;
            /* Commit the inode on the disk */
            ext2_inode_write(f->inode_num, &f->inode);  
        }
        
    }

end: 
    /* Set the file struct properly */
    f->open = 0; 
    f->pos  = 0; 
    f->exist = 0; 
    f->new_bytes_count = 0;
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


/**
 * Ext2 file write. Write size bytes to a file. 
 * @param ext2_file_t * f a pointer to the file to write.
 * @param uint8_t * buff a pointer to the buffer where store the read data.
 * @param uint32_t size the number of bytes to read
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_fwrite( ext2_file_t * f, uint8_t * buff, uint32_t size )   /* TODO here for now we do not buffer anything, all the bytes are written to the disk asap  */
{
    if ( !f->open ) {
        printk("The file is not open.\n");
        return 1;
    }

    /* Check if the file exists on the disk */
    if ( !f->exist ) {   /* the file does not exist, needs to be created */
        if ( ext2_file_create(f->pathname) != 0 ) {
            printk("The file cannot be created for some errors\n");
            return 1;
        } 
        f->exist = 1;
    }

    ext2_super_block * sb = ext2_get_sb();
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_id = f->pos/block_size; 
    uint32_t block_num = f->inode.i_block[block_id];

    /* Want to write beyond the buffer */
    if ( ((f->pos%block_size) + size) > block_size ) {

        memcpy(f->buff+(f->pos%block_size), buff, block_size -  (f->pos%block_size)); 
        ext2_write_block(block_num, f->buff);

        uint32_t allocated_blocks = ext2_sectors_to_blocks(f->inode.i_disk_sectors_count);
        if ( allocated_blocks < ( (f->pos/block_size) + 2 ) ) {  /* Essentially I don't have enough allocated blocks for this file to store also the 4k buffer */

            uint32_t group = (f->inode_num - 1) / sb->s_inodes_per_group;
            /* I have to allocate another block for this file */
            block_num = ext2_free_block_alloc(group) ;

            
            if (block_num == 0) {
                printk("Can't allocate a new block for this file\n");
                return 1;
            }
            
            /* Where to put this new block ? */
            if ( allocated_blocks < EXT2_NDIR_BLOCKS ) {       /* TODO For now we support only direct blocks */
                f->inode.i_block[allocated_blocks++] = block_num;
                f->inode.i_disk_sectors_count += ext2_blocks_to_sectors(1);
            } else {
                /* Indirect blocks... */
                printk("The file is too big\n");
                return 1;  /* TODO Should deallocate the new block */
            }
        } else {
            block_id = f->pos/block_size + 1; 
            block_num = f->inode.i_block[block_id];  /* TODO For now we support only direct blocks */
        }

        /* TODO we are not considering to write more than one block beyond the actual one .... buuuuu */
        
        uint8_t new_buff[block_size]; 
        memset(new_buff, 0x00, block_size);
        memcpy(new_buff, buff+(block_size-(f->pos%block_size)), size+(f->pos%block_size)-block_size);   /* mybe +1 boh */

        printk("");

        ext2_write_block(block_num, new_buff);

    } else { 
        memcpy(f->buff+(f->pos%block_size), buff, size);
        ext2_write_block(block_num, f->buff);
    }


    if ( ((f->pos%block_size) + size) > f->inode.i_lower_size ) { 
        
        f->inode.i_lower_size +=  ((f->pos%block_size) + size) - f->inode.i_lower_size; 
    }
    ext2_inode_write(f->inode_num, &f->inode); 

    return 0; 
}
 
/**
 * Ext2 file seek. Change the position into the file. 
 * @param ext2_file_t * f a pointer to the file to write.
 * @param uint32_t new_pos the new position into the file
 * @return uint8_t non-zero value in case of errors
 */
uint8_t ext2_fseek( ext2_file_t * f, uint32_t new_pos ) 
{
    if ( !f->open ) {
        printk("The file is not open.\n");
        return 1;
    }    

    if ( f->pos == new_pos ) {
        return 0;
    }

    /* Check if the file exists on the disk */
    if ( !f->exist ) {   /* the file does not exist, needs to be created */
        if ( ext2_file_create(f->pathname) != 0 ) {
            printk("The file cannot be created for some errors\n");
            return 1;
        } 
        f->exist = 1;
    }

    ext2_super_block * sb = ext2_get_sb();
    uint32_t block_size = 1024 << sb->s_log_block_size;

    /* The actual buffer of the file is dirty and the new position is in another block, need to be saved on the disk before change the position */
    if ( f->dirty && ((new_pos/block_size) != (f->pos/block_size)) ) { 
        
        uint32_t block_id = f->pos/block_size; 
        uint32_t block_num = f->inode.i_block[block_id];  /* TODO For now we support only direct blocks */
        /* Save the current buffer on the disk */
        ext2_write_block(block_num, f->buff);
        if ( f->new_bytes_count > 0 ) {  /* There are some new bytes to store */
            f->inode.i_lower_size = f->pos+f->new_bytes_count;
            /* Commit the inode on the disk */
            ext2_inode_write(f->inode_num, &f->inode);  
        }
    
    }

    /* Change the position */
    f->pos = new_pos;
    f->new_bytes_count = 0; 
    f->dirty = 0;

    /* First of all need to see if the actual file buffer exists on the disk or need to be created (another block this means) */
    uint32_t allocated_blocks = ext2_sectors_to_blocks(f->inode.i_disk_sectors_count);

    if ( allocated_blocks < ( (f->pos/block_size) + 1 ) ) {  /* Essentially I don't have enough allocated blocks for this file to store also the 4k buffer */

        uint32_t group = (f->inode_num - 1) / sb->s_inodes_per_group;
        /* I have to allocate another block for this file */
        uint32_t block_num = ext2_free_block_alloc(group) ;
        if (block_num == 0) {
            printk("Can't allocate a new block for this file\n");
            return 1;
        }
        
        /* Where to put this new block ? */
        if ( allocated_blocks < EXT2_NDIR_BLOCKS ) {       /* TODO For now we support only direct blocks */
            f->inode.i_block[allocated_blocks++] = block_num;
            f->inode.i_disk_sectors_count += ext2_blocks_to_sectors(1);
        } else {
            /* Indirect blocks... */
            printk("The file is too big\n");
            return 1;  /* TODO Should deallocate the new block */
        }
        ext2_read_block(block_num, f->buff);

        memset(&f->buff, 0x0, f->pos%block_size);
        f->inode.i_lower_size = f->pos%block_size;

        /* Commit the inode on the disk */
        ext2_inode_write(f->inode_num, &f->inode);  

    } else {
        /* Load the block pointed by pos into the buffer */
        uint32_t block_id = f->pos/block_size; 
        uint32_t block_num = f->inode.i_block[block_id];
        ext2_read_block(block_num, f->buff);
    }

    return 0; 
}
 