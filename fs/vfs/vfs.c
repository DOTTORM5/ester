
/* For now statically allocated single file at a time */ 
static vfs_file_t vfs_file; 

vfs_file_t * vfs_file_get(void)
{
	return &vfs_file;
}


uint32_t vfs_mount()
{
	
}

vfs_file_t * vfs_fopen(const char * pathname, const char * mode) 
{
	
	vfs_file_t * vfs_file = vfs_file_get();
	
	if (vfs_file->active) {
		
		printk("File already active\n");
		return NULL; 
		
	}
	
	vfs_file->mode = mode; 
	vfs_file->pos  = 0; 
	vfs_file->active =1; 
	memcpy(vfs_file->pathname, pathname, strlen(pathname));
	// Copy the first 4k of the file into buffer 	
	
}

uint32_t vfs_fclose(vfs_file_t * filestream) 
{
	
}


uint32_t vfs_fread(void * ptr, uint32_t size, uint32_t n, vfs_file_t * filestream)
{
	
	
}

uint32_t vfs_fwrite(const void * ptr, uint32_t size, uint32_t n, vfs_file_t * filestream)
{
	
	
}

uint32_t vfs_fseek(vfs_file_t * filestream, uint64_t offset, uint8_t whence)
{
	
	
}