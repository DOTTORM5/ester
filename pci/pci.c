/* This source file implement all the basic function to access classic (no memory mapped) pci configuration space through mechanism 1 */

#include "pci.h"
#include "utils.h"
#include "debug.h"

/* This containts all pci_devices in terms of bus|dev|func, this should be a vector dinamically allocated, but we don't have heap strategy implemented yet, we wrongly give it a fixed size for now */
static struct {
    __pci_dev pci_devices[PCI_MAX_DEVICES];
    __u16 pci_devices_cnt;     /* Effective number of pci_devices */
} pci_vector_devices;

/*****************************************************************************************************************************/

/* Utility */
/* Configuration space address */
/* | Enable (bit31) | Reserved (bit30-24) | Bus Num (bit23-16) | Device Num (bit15-11) | Function Num (bit10-8) | Register offset (bit7-0) */
/* There are a maximum of 256 byte for each PCI device (function) register, last two bit always 00, we access 32 bits */

/*static*/ __u32 pci_read_config_long(__u8 bus, __u8 device, __u8 function, __u8 offset)
{
    __u32 address =  (__u32) (((__u32)bus) << 16) | (((__u32)device) << 11 ) | (((__u32)function) << 8 ) | (offset & 0xFC) | ((__u32) 0x80000000);
    outl(PCI_CONFIG_ADDRESS_PORT, address);
    return inl(PCI_CONFIG_DATA_PORT);
}

/*static*/ __u16 pci_get_vendor_id(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u16) (pci_read_config_long(bus, device, function, 0x00) & 0x0000FFFF) );
}

/*static*/ __u8 pci_get_header_type(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u8) ((pci_read_config_long(bus, device, function, 0x0C) & 0x00FF0000) >> 16));
}

/*static*/ __u8 pci_get_class_code(__u8 bus, __u8 device, __u8 function)
{
    return ((__u8) ( ( pci_read_config_long(bus,device,function, 0x08) & 0xFF000000)  >> 24));
}

/*static*/ __u8 pci_get_sub_class_code(__u8 bus, __u8 device, __u8 function)
{
    return ((__u8) ( ( pci_read_config_long(bus,device,function, 0x08) & 0x00FF0000)  >> 16));
}

/*static*/ __u8 pci_get_secondary_bus(__u8 bus, __u8 device, __u8 function)
{
    return ( (__u8) ((pci_read_config_long(bus, device, function, 0x18 ) & 0x0000FF00) >> 8));
}

/*****************************************************************************************************************************/

/* PCI structures management */
void pci_vector_dev_init(void)
{
    pci_vector_devices.pci_devices_cnt = 0;
    return;
}

/*static*/ __pci_dev pci_dev_create(__u8 bus, __u8 device, __u8 function)
{
    __pci_dev pci_dev = {bus, device, function};
    return pci_dev;
}

__u16 pci_vector_dev_cnt(void)
{
    return pci_vector_devices.pci_devices_cnt;
}

__pci_dev pci_vector_dev_get(__u16 index)
{
    __u16 cnt = pci_vector_dev_cnt();
    if ( cnt == 0 ) {
        return pci_dev_create(0,0,0);
    }
    if ( index > cnt-1 ) {
        return pci_vector_devices.pci_devices[cnt-1];
    }
    if ( index < 0 ) {
        return pci_vector_devices.pci_devices[0];
    }
    return pci_vector_devices.pci_devices[index];
}

__u8 pci_vector_dev_add(__u8 bus, __u8 device, __u8 function)
{
    __u16 cnt = pci_vector_dev_cnt();
    if ((cnt >= PCI_MAX_DEVICES) || (cnt < 0)) {
        return PCI_VECTOR_DEV_ADD_ERR;
    }
    pci_vector_devices.pci_devices[pci_vector_devices.pci_devices_cnt++] = pci_dev_create(bus, device, function);
    return PCI_VECTOR_DEV_ADD_SUCC;
}


/*****************************************************************************************************************************/

/* PCI scanning */
/* We're assuming that UEFI BIOS already configured PCI-to-PCI bridges */
/* We adopt a recursive scan, well exploring the PCI tree */

/*static*/ void pci_function_scan(__u8 bus, __u8 device, __u8 function)
{
    __u8 class_code;
    __u8 sub_class_code;

    class_code = pci_get_class_code(bus, device, function);
    sub_class_code = pci_get_sub_class_code(bus, device, function);
    if ( (class_code ==  0x06) && (sub_class_code == 0x04) ) {  /* This is a bridge */
        pci_bus_scan(pci_get_secondary_bus(bus, device, function));
    }
    /* Here a device in terms of bus|device|function is scanned, We can add it to the list of devices */
    if ( pci_vector_dev_add(bus, device, function) == PCI_VECTOR_DEV_ADD_ERR ) {
        DEBUG_ERROR("Problem adding PCI device");
        return;
    }
    return;
}

/*static*/ void pci_device_scan(__u8 bus, __u8 device)
{
    __u8 function = 0;
    if ( pci_get_vendor_id(bus, device, function) == PCI_NO_VENDOR ) return;
    pci_function_scan(bus, device, function);
    if (pci_get_header_type(bus, device, function) & 0x80) { /* There are multiple functions */
        for (function = 1; function < PCI_MAX_FUNCTIONS_PER_DEVICE; function++) {
            if( pci_get_vendor_id(bus, device, function) != PCI_NO_VENDOR ) {
                pci_function_scan(bus, device, function);
            }
        }
    }
    return;
}

/*static*/ void pci_bus_scan(__u8 bus)
{
    for (__u8 i = 0; i < PCI_MAX_DEVICES_PER_BUS; i++ ) {
        pci_device_scan(bus, i);
    }
    return;
}

void pci_recursive_scan(void)
{
    /* Initialize the PCI vector device structure */
    pci_vector_dev_init(); /* Maybe this has to be called externally ? */

    /* Start from root complex, always 0,0,0 */
    if ( (pci_get_header_type(0,0,0) & 0x80) == 0 ) {   /* Check if has multiple functions, the LSB of header type says this */
        /* Only one function */
        if (pci_get_vendor_id(0,0,0) == PCI_NO_VENDOR) return; /* There is no device, something went wrong, it means there is no PCI tree */
        pci_bus_scan(0);
    } else {
        /* Host root complex has multiple functions, i.e. there are multiple pci host controllers */
        for (__u8 i = 0; i < PCI_MAX_FUNCTIONS_PER_DEVICE; i++){ /* Scan all function */
            if (pci_get_vendor_id(0,0,i) == PCI_NO_VENDOR) break;  /* Function not present, stop */
            pci_bus_scan(i); /* Here the function is treated as a bus */
        }
    }
    return;
}

/*****************************************************************************************************************************/