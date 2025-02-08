#ifndef ELF64_LOADER_H
#define ELF64_LOADER_H

#include "types.h"

#define ELF_NIDENT	16
#define ELF_MAGIC   0x7F454C46         /* The ELF magic number 0x7F 'ELF' */

typedef struct {
    uint8_t    e_ident[ELF_NIDENT];    /* ELF identifier                                   */
    uint16_t   e_type;                 /* ELF type                                         */
	uint16_t   e_machine;              /* ISA                                              */
	uint32_t   e_version;              /* ELF version                                      */
	uint64_t   e_entry;                /* Program entry point                              */
	uint64_t   e_phoff;                /* Program header table offset                      */
	uint64_t   e_shoff;                /* Section header table offset                      */
	uint32_t   e_flags;                /* Flags - architecture dependent                   */
	uint16_t   e_ehsize;               /* ELF header size                                  */
	uint16_t   e_phentsize;            /* Program header entry size                        */
 	uint16_t   e_phnum;                /* Number of entries in the program header table    */
	uint16_t   e_shentsize;            /* Section header entry size                        */
	uint16_t   e_shnum;                /* Number of entries in the section header table    */
	uint16_t   e_shstrndx;             /* Section index to the section header string table */

} __elf64_header; 



typedef struct {
	uint32_t p_segtype;     /* Type of segment                                                      */
	uint32_t p_flags;       /* Flags                                                                */
	uint64_t p_offset;      /* The offset in the file that the data for this segment can be found   */
	uint64_t p_vaddr;       /* Where you should start to put this segment in virtual memory         */
	uint64_t p_paddr;       /* Reserved for segment's physical address                              */ 
	uint64_t p_filesz;      /* Size of the segment in the file                                      */
	uint64_t p_memsz;       /* Size of the segment in memory (p_memsz, at least as big as p_filesz) */
	uint64_t p_align;       /* The required alignment for this section (usually a power of 2)       */ 
} __elf64_program_header; 



__elf64_header * elf64_get_header(void); 
void elf64_extract_header (uint8_t * ptr);  
void elf64_load ( char * file_name ); 

#endif // ELF64_LOADER_H