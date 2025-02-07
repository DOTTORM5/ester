#ifndef ELF64_LOADER_H
#define ELF64_LOADER_H

#define ELF_NIDENT	16

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



#endif // ELF64_LOADER_H