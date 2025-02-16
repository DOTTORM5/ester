#include "elf64_loader.h"
#include "mem.h"
#include "ext2.h"
#include "printk.h"

#define BASE_VMEM 2147483648     /* 2 GB ... TODO very ugly */

static elf64_header_t elf64_header;
static elf64_program_header_t elf64_program_headers[128]; 

/**
 * Elf64 get the current elf64_header. 
 * @param void
 * @return elf64_header_t * a pointer to the current elf64_header
 */
elf64_header_t * elf64_get_header(void)
{
    return &elf64_header;
} 

/**
 * Elf64 get an entry in the program headers table. 
 * @param uint16_t index the indez in the program headers table
 * @return elf64_program_header_t * a pointer to the required program header
 */
elf64_program_header_t * elf64_get_program_header(uint16_t index)
{
    return &elf64_program_headers[index];
}

/**
 * Elf64 header extractor. Save the elf header in memory 
 * @param uint8_t * ptr a pointer to the start of the header in a file
 * @return void
 */
void elf64_extract_header (uint8_t * ptr) 
{
    elf64_header_t * elf64_header = elf64_get_header(); 
    memcpy(elf64_header, ptr, sizeof(elf64_header_t)); 
    return;
}

/**
 * Elf64 program header extractor. Save the elf program header in the array of program headers 
 * @param uint8_t * ptr a pointer to the start of the header in a file
 * @param uint16_t index the index in the program headers table where to save the program header
 * @return void
 */
void elf64_extract_program_header (uint8_t * ptr, uint16_t index) 
{
    elf64_program_header_t * elf64_program_header = elf64_get_program_header(index); 
    memcpy(elf64_program_header, ptr, sizeof(elf64_program_header_t)); 
    return;
}


void stub (void)
{
    printk("NELLA CUSCINETTO\n");
    return;

} 

/**
 * Elf64 program loader. Load the program in memory ready to be executed! 
 * @param char * file_name the name of the file of the program to be loaded
 * @return void
 */
void elf64_load ( char * file_name )
{
    uint8_t buff[4096];
    // ext2_read_file(file_name, 0, buff);
    
    elf64_extract_header(buff);
    elf64_header_t * elf64_header = elf64_get_header();

    if ( ! ((uint32_t)elf64_header->e_ident & ELF_MAGIC) ) {
        printk("Not a valid ELF\n");
        return;
    }

    /* For now, we consider that the entire program is in the first file block (4096 bytes) */
    uint64_t offset = elf64_header->e_phoff;
    for ( uint16_t i = 0; i<elf64_header->e_phnum; i++ ) {
        elf64_extract_program_header(buff+offset, i);
        offset += elf64_header->e_phentsize; 
    }


    printk("Entries count %d\n", elf64_header->e_phnum);

    elf64_program_header_t * elf64_program_header = elf64_get_program_header(1);

    printk("Program header p_segtype: %d\n", elf64_program_header->p_segtype);
    printk("Program header p_flags: %d\n", elf64_program_header->p_flags);
    printk("Program header p_offset: %d\n", elf64_program_header->p_offset);
    printk("Program header p_vaddr: %d\n", elf64_program_header->p_vaddr);
    printk("Program header p_paddr: %d\n", elf64_program_header->p_paddr);
    printk("Program header p_filesz: %d\n", elf64_program_header->p_filesz);
    printk("Program header p_memsz: %d\n", elf64_program_header->p_memsz);
    printk("Program header p_align: %d\n", elf64_program_header->p_align);

    uint8_t * data; 

    for ( uint16_t i = 0; i<elf64_header->e_phnum; i++ ) {
        // ext2_read_file(file_name, i, buff);
        elf64_program_header = elf64_get_program_header(i);
        pmap(elf64_program_header->p_vaddr+BASE_VMEM, elf64_program_header->p_vaddr, 0); 
        if ( elf64_program_header->p_segtype == 1 ) { 
            memset((uint8_t *)elf64_program_header->p_vaddr, 0x00, elf64_program_header->p_memsz);  /* Clean the memory */
            memcpy((uint8_t *)elf64_program_header->p_vaddr, buff/*+elf64_program_header->p_offset*/, elf64_program_header->p_filesz);
            data = (uint8_t * ) elf64_program_header->p_vaddr; 
            printk("VADDR: %d\n", elf64_program_header->p_vaddr);
            printk("DATA: %d\n", *data);
            printk("BUFF: %d\n", buff[0]);
        }
    }

    printk("Entry point: %d\n", elf64_header->e_entry);

    // data = *(uint64_t * ) 0x401000; 

    printk("READ AT ENTRY: %d\n", data);


    __asm__("pushq $0x000000000010015c");
    __asm__("jmp 0x401000"); 

    printk("AFTER JUMPING!!!\n");

    return; 
}


