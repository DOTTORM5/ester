#ifndef VFS_H
#define VFS_H

typedef struct {
	uint8_t active; 
	char mode [2];    /* File mode */
	char pathname[MAX];
	uint64_t pos;     /* The actual file position */ 
	char buff[4096]; /* A buffer of 4k of the file from pos */ 
} vfs_file_t; 


#endif