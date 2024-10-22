/* This source file implement all the basic function to access classic (no memory mapped) pci configuration space through mechanism 1 */

#include "pci.h"
#include "utils.h"
#include "debug.h"
#include "printk.h"

/* This containts all pci_devices in terms of bus|dev|func, this should be a vector dinamically allocated, but we don't have heap strategy implemented yet, we wrongly give it a fixed size for now */
static struct {
    __pci_dev pci_devices[PCI_MAX_DEVICES];
    uint16_t pci_devices_cnt;     /* Effective number of pci_devices */
} pci_vector_devices;

/*****************************************************************************************************************************/

/* Utility */
/* Configuration space address */
/* | Enable (bit31) | Reserved (bit30-24) | Bus Num (bit23-16) | Device Num (bit15-11) | Function Num (bit10-8) | Register offset (bit7-0) */
/* There are a maximum of 256 byte for each PCI device (function) register, last two bit always 00, we access 32 bits */

/*static*/ uint32_t pci_read_config_long(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t address =  (uint32_t) (((uint32_t)bus) << 16) | (((uint32_t)device) << 11 ) | (((uint32_t)function) << 8 ) | (offset & 0xFC) | ((uint32_t) 0x80000000);
    outl(PCI_CONFIG_ADDRESS_PORT, address);
    return inl(PCI_CONFIG_DATA_PORT);
}

/*static*/ uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function)
{
    return ( (uint16_t) (pci_read_config_long(bus, device, function, 0x00) & 0x0000FFFF) );
}

/*static*/ uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function)
{
    return ( (uint8_t) ((pci_read_config_long(bus, device, function, 0x0C) & 0x00FF0000) >> 16));
}

/*static*/ uint8_t pci_get_class_code(uint8_t bus, uint8_t device, uint8_t function)
{
    return ((uint8_t) ( ( pci_read_config_long(bus,device,function, 0x08) & 0xFF000000)  >> 24));
}

/*static*/ uint8_t pci_get_sub_class_code(uint8_t bus, uint8_t device, uint8_t function)
{
    return ((uint8_t) ( ( pci_read_config_long(bus,device,function, 0x08) & 0x00FF0000)  >> 16));
}

/*static*/ uint8_t pci_get_prog_if_code(uint8_t bus, uint8_t device, uint8_t function)
{
    return ((uint8_t) ( ( pci_read_config_long(bus,device,function, 0x08) & 0x0000FF00)  >> 8));
}

/*static*/ uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function)
{
    return ( (uint8_t) ((pci_read_config_long(bus, device, function, 0x18 ) & 0x0000FF00) >> 8));
}

/*****************************************************************************************************************************/

/* PCI structures management */
void pci_vector_dev_init(void)
{
    pci_vector_devices.pci_devices_cnt = 0;
    return;
}

/*static*/ __pci_dev pci_dev_create(uint8_t bus, uint8_t device, uint8_t function)
{
    __pci_dev pci_dev = {bus, device, function};
    return pci_dev;
}

uint16_t pci_vector_dev_cnt(void)
{
    return pci_vector_devices.pci_devices_cnt;
}

__pci_dev pci_vector_dev_get(uint16_t index)
{
    uint16_t cnt = pci_vector_dev_cnt();
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

uint8_t pci_vector_dev_add(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t cnt = pci_vector_dev_cnt();
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

/*static*/ void pci_function_scan(uint8_t bus, uint8_t device, uint8_t function)
{
    uint8_t class_code;
    uint8_t sub_class_code;

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

/*static*/ void pci_device_scan(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;
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

/*static*/ void pci_bus_scan(uint8_t bus)
{
    for (uint8_t i = 0; i < PCI_MAX_DEVICES_PER_BUS; i++ ) {
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
        for (uint8_t i = 0; i < PCI_MAX_FUNCTIONS_PER_DEVICE; i++){ /* Scan all function */
            if (pci_get_vendor_id(0,0,i) == PCI_NO_VENDOR) break;  /* Function not present, stop */
            pci_bus_scan(i); /* Here the function is treated as a bus */
        }
    }
    return;
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data) {
    // Construct the PCI configuration address
    uint32_t address = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | ((uint32_t)function << 8) | (offset & 0xFC);

    // Write the address to the PCI configuration address port
    outl(0xCF8, address);

    // Write the data to the PCI configuration data port
    outl(0xCFC, data);
}

/* TO PUT IN ANOTHER FILE */
void pci_ahci_init(uint8_t bus, uint8_t slot, uint8_t func)
{
    // Read the current PCI configuration register (command register is at offset 0x04)
    uint16_t command = pci_read_config_long(bus, slot, func, 0x04);

    // Enable interrupts (bit 10), DMA (bit 8), and memory space access (bit 1)
    command |= (1 << 10);  // Set bit 10 for interrupts
    command |= (1 << 8);   // Set bit 8 for DMA
    command |= (1 << 2);   // Set bit 2 Bus mastering
    command |= (1 << 1);   // Set bit 1 for memory space access
    

    // Write back the modified command register value
    pci_write(bus, slot, func, 0x04, command);

    return;
}


/* FIND THE AHCI Controller and return the ABAR. BAR[5], absolutely handle this in a better way!!! */
uint32_t pci_ahci_get_abar(void)
{
    uint8_t class_id    = 0; 
    uint8_t subclass_id = 0;
    uint8_t prog_if = 0;
    uint32_t bar = 0;
    for ( uint8_t i = 0; i < pci_vector_devices.pci_devices_cnt; i++ ) {
        class_id = pci_get_class_code(pci_vector_devices.pci_devices[i].bus, pci_vector_devices.pci_devices[i].device, pci_vector_devices.pci_devices[i].function );
        subclass_id = pci_get_sub_class_code(pci_vector_devices.pci_devices[i].bus, pci_vector_devices.pci_devices[i].device, pci_vector_devices.pci_devices[i].function );
		prog_if = pci_get_prog_if_code(pci_vector_devices.pci_devices[i].bus, pci_vector_devices.pci_devices[i].device, pci_vector_devices.pci_devices[i].function);
        // printk("CLASS ID: %d\nSUBCLASS ID %d\nPROGIF: %d\n", class_id, subclass_id, prog_if);
        if ( class_id == 0x1 && subclass_id == 0x6 ) {
            bar = pci_read_config_long(pci_vector_devices.pci_devices[i].bus, pci_vector_devices.pci_devices[i].device, pci_vector_devices.pci_devices[i].function, 0x24);
            pci_ahci_init(pci_vector_devices.pci_devices[i].bus, pci_vector_devices.pci_devices[i].device, pci_vector_devices.pci_devices[i].function);
            return bar;
		} 
	}
}





/*****************************************************************************************************************************/