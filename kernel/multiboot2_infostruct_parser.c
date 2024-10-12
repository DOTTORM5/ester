#include "multiboot2_infostruct_parser.h"
#include "debug.h"


/* For now it just goes for the type=3 tag that is the tag of our loaded module and returns a pointer to it */
__module_tag * multiboot2_parser (uint32_t base_address)
{
    __fixed_tag  * fixed_tag = (__fixed_tag *) base_address;
    uint32_t * u32_ptr = ( uint32_t * ) base_address;
    __module_tag * module_tag;
    uint32_t cnt = 0;

    while(cnt < fixed_tag->total_size){
        if ( *u32_ptr == 0x3) {
            module_tag = (__module_tag *) u32_ptr;
            break;
        }
        u32_ptr++;
        cnt += 4;
    }

    return module_tag;
}