/* This code is inspired by OSDev.org and Linux kernel.org */

#ifndef EXT2_H
#define EXT2_H 

#include "types.h"

/* Filename max length */
#define EXT2_NAME_LEN 255

/* Data blocks constants for inode */
#define	EXT2_NDIR_BLOCKS        12                        /* Number of direct blocks          */
#define	EXT2_IND_BLOCK          EXT2_NDIR_BLOCKS          /* Number of indirect blocks        */
#define	EXT2_DIND_BLOCK         (EXT2_IND_BLOCK + 1)      /* Number of doubly indirect blocks */
#define	EXT2_TIND_BLOCK         (EXT2_DIND_BLOCK + 1)     /* Number of triply indirect blocks */
#define	EXT2_N_BLOCKS           (EXT2_TIND_BLOCK + 1)     /* Total number of blocks           */

/* ext2 superblock structure */
typedef struct {
    uint32_t s_inodes_count;                 /* Number of inodes in the fs                              */
    uint32_t s_blocks_count;                 /* Number of blocks in the fs                              */
	uint32_t s_r_blocks_count;               /* Number of blocks reserved for superuser                 */
    uint32_t s_free_blocks_count;            /* Number of unallocated blocks                            */
    uint32_t s_free_inodes_count;            /* Number of unallocared inodes                            */
    uint32_t s_block_sb;                     /* Number of the block containing the superblock           */ 
    uint32_t s_log_block_size;               /* log2(block_size) - 10                                   */
    uint32_t s_log_frag_size;                /* log2(fragment_size) - 10                                */
    uint32_t s_blocks_per_group;             /* Number of blocks in a group                             */
    uint32_t s_frags_per_group;              /* Number of fragments in a group                          */
	uint32_t s_inodes_per_group;             /* Number of inodes in a group                             */	
    uint32_t s_mtime;		                 /* Last Mount time                                         */
	uint32_t s_wtime;		                 /* Last Write time                                         */
    uint16_t s_mnt_count;		             /* Mount count                                             */
	uint16_t s_max_mnt_count;	             /* Maximal mount count                                     */
	uint16_t s_magic;		                 /* Magic signature                                         */
    uint16_t s_state;		                 /* File system state                                       */
	uint16_t s_errors;		                 /* Behaviour when detecting errors                         */
	uint16_t s_minor_rev_level; 	         /* minor revision level                                    */
	uint32_t s_lastcheck;		             /* time of last check                                      */
	uint32_t s_checkinterval;	             /* max. time between checks                                */
	uint32_t s_creator_os;		             /* OS                                                      */
	uint32_t s_rev_level;		             /* Revision level                                          */
	uint16_t s_def_resuid;		             /* Default user id for reserved blocks                     */
	uint16_t s_def_resgid;		             /* Default group id for reserved blocks                    */
    uint32_t s_first_free_inode;             /* First non-reserved inode in the fs                      */
    uint16_t s_inode_size;                   /* Inode size, in versions > 0 could be different from 128 */
	
    /* other fields not implemented for now */
    /* WARNING - all the value in the struct must be little endian */
} ext2_super_block_t;

/* ext2 block group descriptor structure */
typedef struct {
    uint32_t bg_block_bitmap;                /* Block containing the block usage bitmap */
    uint32_t bg_inode_bitmap;                /* Block containing the inode usage bitmap */
    uint32_t bg_inode_table;                 /* Starting block of the inode table       */
    uint16_t bg_free_blocks_count;           /* Free blocks in the group                */
    uint16_t bg_free_inodes_count;           /* Free inodes in the group                */
    uint16_t bg_used_dirs_count;             /* Directories in the group                */
    uint16_t bg_padding;                     /* Unused                                  */
    uint32_t bg_unused[3];                   /* Unused                                  */

    /* WARNING - all the value in the struct must be little endian */
} ext2_block_group_descriptor_t;

/* ext2 directory entry structure */
typedef struct {
    uint32_t dir_inode;                      /* The inode index where to find the dir content                */
    uint16_t dir_entry_size;                 /* The total size of this entry                                 */
    uint8_t  dir_name_len;                   /* Name Length least-significant 8 bits                         */
    uint8_t  dir_type;                       /* Type indicator (WARNING - Not always present, check the doc) */
    uint8_t  dir_name[];                     /* Dir name                                                     */ 

    /* WARNING - all the value in the struct must be little endian */
} ext2_dir_entry_t; 


/* ext2 inode structure */
typedef struct {
    uint16_t i_type_permission;             /* Type and permission                                    */
    uint16_t i_user_id;                     /* Owner User ID                                          */
    uint32_t i_lower_size;                  /* Lower 32 bits of size in bytes                         */
    uint32_t i_last_access;                 /* Last access time                                       */
    uint32_t i_creation;                    /* Creation time                                          */
    uint32_t i_last_mod;                    /* Last modification time                                 */ 
    uint32_t i_del;                         /* Deletion time                                          */
    uint16_t i_group_id;                    /* Owner Group ID                                         */
    uint16_t i_links_count;                 /* Count of hard links (directory entries) to this inode  */
    uint32_t i_disk_sectors_count;          /* Count of disk sectors in use by this inode             */
    uint32_t i_flags;                       /* Flags                                                  */
    uint32_t i_os_specific_1;               /* OS specific value 1                                    */
    uint32_t i_block[EXT2_N_BLOCKS];        /* Block pointers                                         */
    uint32_t i_gen_number;                  /* Generation number                                      */
    uint32_t i_ext_att_block;               /* Extended attribute block (not used for v0) or File ACL */
    uint32_t i_dir_acl;                     /* Directory ACL                                          */
    uint32_t i_frag_block;                  /* Block address of fragment                              */
    uint32_t i_os_specific_2[3];            /* OS specific value 2                                    */
} ext2_inode_t;

uint8_t ext2_extract_sb (void /*ext2_super_block_t * sb*/);
uint32_t ext2_get_block_size(void);
uint32_t ext2_get_inodes_per_group(void);
uint8_t ext2_extract_bgdt(void);
uint32_t ext2_get_inode_size(void);
ext2_block_group_descriptor_t * ext2_get_bgd(uint32_t group_num);


/* ext2 directory entry structure */
typedef struct {
    uint32_t dir_inode;                      /* The inode index where to find the dir content                */
    uint16_t dir_entry_size;                 /* The total size of this entry                                 */
    uint8_t  dir_name_len;                   /* Name Length least-significant 8 bits                         */
    uint8_t  dir_type;                       /* Type indicator (WARNING - Not always present, check the doc) */
    uint8_t  dir_name[EXT2_NAME_LEN+1];      /* Dir name fixed now                                           */ 

    /* WARNING - all the value in the struct must be little endian */
} ext2_dir_entry_fixed_name_t; 

ext2_inode_t * ext2_get_current_inode(void);
ext2_inode_t * ext2_extract_inode(uint32_t inode_num);
uint32_t ext2_free_inode_alloc( uint32_t group );
uint32_t ext2_free_block_alloc( uint32_t group );
void ext2_inode_write(uint32_t inode_num, ext2_inode_t * inode);
uint32_t ext2_inode_find ( const char * pathname ) ;


/* Logical Directory structures - TODO implement differently to support also other fs */
#define MAX_SUBDIRS    128     /* MAX number of subdirs and files in a directory */
#define MAX_PATH_LEN   4096    /* MAX path total char length */
#define MAX_PATH_DEPTH 10      /* MAX level in a path */

#define BUFF_SIZE 4096        /* The buffer size of a file */

/* This could be extend to include all the field associated to a directory... TODO */
typedef struct {
    uint32_t cwd_inode_number;
    char cwd_name[MAX_PATH_LEN];
} cwd_t;

uint16_t ext2_list_directory(uint32_t dir_inode_num, ext2_dir_entry_fixed_name_t * dir_entries);
void ext2_init_cwd(void);
void ext2_change_cwd(const char * cwd_name);
uint8_t ext2_add_dir_entry (ext2_inode_t * dir_inode, uint32_t new_entry_inode_num, const char * new_entry_name);
uint16_t unpack_dir_path(const char * dir_path, char unpacked_path[MAX_PATH_DEPTH][EXT2_NAME_LEN+2]);

typedef struct {
    uint32_t inode_num;             /* The inode number of the file */
    const char mode[2];             /* The file mode, read, write etc. */
    uint8_t open;                   /* If the file is open */
    uint32_t pos;                   /* The byte position actual accessed */
    uint32_t new_bytes_count;        /* Number of new bytes from pos */
    uint8_t dirty;                  /* The file is dirty, need to be saved on the disk */
    uint8_t exist;                  /* The file exist on the disk */
    char pathname[MAX_PATH_LEN];    /* The pathname of the file */ 
    char buff[BUFF_SIZE];           /* Buffer the file content */ 
    ext2_inode_t inode;             /* The inode of the file */

} ext2_file_t; 

ext2_file_t * ext2_fopen( const char * pathname, const char * mode ) ;
uint8_t ext2_fclose( ext2_file_t * f ) ;
uint8_t ext2_fwrite( ext2_file_t * f, uint8_t * buff, uint32_t size );
uint8_t ext2_fseek( ext2_file_t * f, uint32_t new_pos ) ;
uint8_t ext2_fread( ext2_file_t * f, uint8_t * buff, uint32_t size ) ;


uint8_t ext2_read_block( uint32_t block_id, uint8_t * buff ); 
uint8_t ext2_write_block( uint32_t block_id, uint8_t * buff );

uint8_t ext2_write_bgdt(void);


#endif // EXT2_H 