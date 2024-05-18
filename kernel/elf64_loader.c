#include "elf64_loader.h"

/* For now the elf64 is assumed to be in memory */
/* Steps to do:
    1. Prepare pagetables to allocate a memory space for the program
    2. Load the program to that memory space
    3. Store the mapping virtual/physical for this process to be executed to restore it later
    4. All the stuff needed for task management, i don't remember this now
    5. Start the process
*/
void elf64_loads ( __u64 base_address, __u32 total_size )
{


    return; 
}