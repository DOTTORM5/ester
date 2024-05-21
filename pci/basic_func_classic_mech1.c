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

static __u16 pci_get_vendor_id(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u16) (pci_read_config_long(bus, device, function, 0x00) & 0x0000FFFF) );
}

static __u8 pci_get_header_type(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u8) ((pci_read_config_long(bus, device, function, 0x0C) & 0x00FF0000) >> 16));
}

static __u8 pci_get_secondary_bus(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u8) ((pci_read_config_long(bus, device, function, 0x18 ) & 0x0000FF00) >> 8));
}

/* PCI scanning */
/* We're assuming that UEFI BIOS already configured PCI-to-PCI bridges */
/* We adopt a recursive scan, well exploring the PCI tree */

void pci_scan_function(__u8 bus, __u8 device, __u8 function)
{
    /* To be implemented */
}

void pci_scan_device(__u8 bus, __u8 device)
{
    __u8 function = 0;
    if ( pci_get_vendor_id(bus, device, function) == PCI_NO_VENDOR ) return;
    pci_scan_function(bus, device, function);
    if (pci_get_header_type(bus, device, function) & 0x80) { /* There are multiple functions */
        for (function = 1; function < PCI_MAX_FUNCTIONS_PER_DEVICE; function++)  {
            if( pci_get_vendor_id(bus, device, function) != PCI_NO_VENDOR ){
                pci_scan_function(bus, device, function);
            }
        }
    }
}

void pci_scan_bus(__u8 bus)
{
    for (__u8 i = 0; i < PCI_MAX_DEVICES_PER_BUS; i++ ) {
        pci_scan_device(bus, i);
    }
}

void pci_recursive_scan(void)
{
    /* Start from root complex, always 0,0,0 */
    if ( (pci_get_header_type(0,0,0) & 0x80) == 0 ) {   /* Check if has multiple functions, the LSB of header type says this */
        /* Only one function */
        if (pci_get_vendor_id(0,0,0) == PCI_NO_VENDOR) return; /* There is no device, something went wrong, it means there is no PCI tree */
        pci_scan_bus(0);
    } else {
        /* Host root complex has multiple functions, i.e. there are multiple pci host controllers */
        for (__u8 i = 0; i < PCI_MAX_FUNCTIONS_PER_DEVICE; i++){ /* Scan all function */
            if (pci_get_vendor_id(0,0,i) == PCI_NO_VENDOR) break;  /* Function not present, stop */
            pci_scan_bus(i); /* Here the function is treated as a bus */
        }
    }
}
