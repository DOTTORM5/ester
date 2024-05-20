/* This source file implement all the basic function to access classic (no memory mapped) pci configuration space through mechanism 1 */

#include "pci.h"
#include "utils.h"

/* Configuration space address */
/* | Enable (bit31) | Reserved (bit30-24) | Bus Num (bit23-16) | Device Num (bit15-11) | Function Num (bit10-8) | Register offset (bit7-0) */
/* There are a maximum of 256 byte for each PCI device (function) register, last two bit always 00, we access 32 bits */
static __u32 pci_read_config_long(__u8 bus, __u8 device, __u8 function, __u8 offset)
{
    __u32 address =  (__u32) (((__u32)bus) << 16) | (((__u32)device) << 11 ) | (((__u32)function) << 8 ) | (offset & 0xFC) | ((__u32) 0x80000000);
    outl(PCI_CONFIG_ADDRESS_PORT, address);
    return inl(PCI_CONFIG_DATA_PORT);
}

