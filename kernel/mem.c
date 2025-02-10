#include "mem.h"
#include "printk.h"

#define PAGE_PRESENT  0x1
#define PAGE_WRITABLE 0x2
#define PAGE_USER     0x4
#define PAGE_SIZE_4KB 0x0
#define PAGE_SIZE_2MB 0x80

/**
 * Memory copy. Copy len bytes from src to dst.
 * @param void * dst a pointer to the destination memory.
 * @param const void * src a pointer to the source memory.
 * @param uint32_t len the number of bytes to be copied.
 * @return void.
 */
void memcpy(void * dst, const void * src, uint32_t len)
{
    uint8_t * d = (uint8_t *) dst;
    const uint8_t * s = (const uint8_t *) src;  
    for ( uint32_t i = 0; i < len; i++ ) {
        d[i] = s[i];
    }
    return;
}

/**
 * Memory set. Set len bytes pointed by ptr with val.
 * @param void * ptr a pointer to the memory space to be set.
 * @param uint8_t val the value to set the memory space. 
 * @param uint32_t len the number of bytes to be set.
 * @return void. 
 */
void memset(void * ptr, uint8_t val, uint32_t len)
{ 
    uint8_t * tmp =  (uint8_t *) ptr;
    for ( uint32_t i = 0; i < len; i++ ) {
        *tmp = val;
    }
    return;
}

/** 
 * Read cr3. Read the value in the CR3 register to get the start address of page table.
 * @param void.
 * @return uint64_t the value in CR3.
 */
static inline uint64_t read_cr3(void) {
    uint64_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

/** 
 * Page alloc. For now this routine simply walks through the actual page table (from baddr) to find an address that is free (has not PAGE_PRESENT flag).
 * @param uint64_t * baddr (base address) the address where to start to find a valid page.
 * @return uint64_t * a pointer to the first suitable address. 
 */
uint64_t * palloc ( uint64_t * baddr ) 
{
    uint64_t i = 0;
    while ( baddr[i] & PAGE_PRESENT ) {
        i++; 
    }
    return baddr+i;
}

/**
 * Page map. Map a physical address to a virtual address page. 
 * @param uint64_t paddr the physical address.
 * @param uint64_t vaddr the virtual address.
 * @param uint64_t flags the flag to use when init the page, 0 is good.
 * @return void.
 */ 
void pmap(uint64_t paddr, uint64_t vaddr, uint64_t flags)
{
    /* For now we adopt a very basic implementation... need to optimize this */
    /* Indices for each level of the page table */ 
    uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_index  = (vaddr >> 30) & 0x1FF;
    uint64_t pd_index   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_index   = (vaddr >> 12) & 0x1FF;

    uint64_t * pml4_base  = (uint64_t *) read_cr3(); 

    /* These are needed to find another free page in the case the page is not present, these addresses are the ones defined in the boot.asm */
    uint64_t * pdp_physical_base = pml4_base+512; 
    uint64_t * pd_physical_base  = pdp_physical_base+512;
    uint64_t * pt_physical_base  = pd_physical_base+(512*6);

    /* Get the PML4 entry */
    uint64_t* pdp_base;
    if (!(pml4_base[pml4_index] & PAGE_PRESENT)) {
        pdp_base = (uint64_t*) palloc(pdp_physical_base); // Allocate a new page for PDP table
        pml4_base[pml4_index] = ((uint64_t)pdp_base) | PAGE_PRESENT | PAGE_WRITABLE | flags;
    } else {
        pdp_base = (uint64_t*)(pml4_base[pml4_index] & ~0xFFF);
    }

    /* Get the PDP entry */
    uint64_t* pd_base;
    if (!(pdp_base[pdp_index] & PAGE_PRESENT)) {
        pd_base = (uint64_t*) palloc(pd_physical_base); // Allocate a new page for PD table
        pdp_base[pdp_index] = ((uint64_t)pd_base) | PAGE_PRESENT | PAGE_WRITABLE | flags;
    } else {
        pd_base = (uint64_t*)(pdp_base[pdp_index] & ~0xFFF);
    }

    /* Get the PD entry */ 
    uint64_t* pt_base;
    if (!(pd_base[pd_index] & PAGE_PRESENT)) {
        pt_base = (uint64_t*) palloc(pt_physical_base); // Allocate a new page for PT table
        pd_base[pd_index] = ((uint64_t)pt_base) | PAGE_PRESENT | PAGE_WRITABLE | flags;
    } else {
        pt_base = (uint64_t*)(pd_base[pd_index] & ~0xFFF);
    }

    /* Finally, map the physical address to the virtual address in the page table */
    pt_base[pt_index] = (paddr & ~0xFFF) | PAGE_PRESENT | PAGE_WRITABLE | flags;
    return;
}