#ifndef PCI_H
#define PCI_H

#include "types.h"

/* ALL that follows is for mechanism 1 no memory mapped config space */

/* I/O port address for PCI configuration space in classic mode mechanism 1 */
#define PCI_CONFIG_ADDRESS_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT    0xCFC

/* The is no device if the vendor is all 1s */
#define PCI_NO_VENDOR           0xFFFF

/* Max number of pci devices, this has to be modified when introduce memory dinamic allocation */
#define PCI_MAX_DEVICES               128

/* Max number of bus, device and function (in old IO mapped config space) */
#define PCI_MAX_BUSES                 256
#define PCI_MAX_DEVICES_PER_BUS       32
#define PCI_MAX_FUNCTIONS_PER_DEVICE  8


#define PCI_VECTOR_DEV_ADD_ERR        1
#define PCI_VECTOR_DEV_ADD_SUCC       0

/* Maybe these structures are useless, we need just save what device exists in terms of bus|dev|func, that are the parameter used to access in I/O mechanism 1  */
/* For now unused */
/* General PCI device */
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision_id;
    uint8_t  prog_if;
    uint8_t  subclass_code;
    uint8_t  class_code;
    uint8_t  cache_line_size;
    uint8_t  latency_timer;
    uint8_t  header_type;   /* this is always 0x0 for a general device */
    uint8_t  bist;
    uint32_t bar_0;
    uint32_t bar_1;
    uint32_t bar_2;
    uint32_t bar_3;
    uint32_t bar_4;
    uint32_t bar_5;
    uint32_t cardbus_cis_pointer;  /* to be properly defined, should be a pointer ? */
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base_address;
    uint8_t  capabilities_pointer;  /* to be properly defined, should be a pointer ? */
    uint8_t  reserved_0;
    uint16_t reserved_1;
    uint32_t reserved_2;
    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint8_t  min_grant;
    uint8_t  max_latency;
} general_device_config_space_t; /* should it be aligned ? */

/* PCI-to-PCI bridge */
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision_id;
    uint8_t  prog_if;
    uint8_t  subclass_code;
    uint8_t  class_code;
    uint8_t  cache_line_size;
    uint8_t  latency_timer;
    uint8_t  header_type;   /* this is always 0x1 for a pci-to-pci bridge */
    uint8_t  bist;
    uint32_t bar_0;
    uint32_t bar_1;
    uint8_t  primary_bus_number;
    uint8_t  secondary_bus_number;
    uint8_t  subordinate_bus_number;
    uint8_t  secondary_latency_timer;
    uint8_t  io_base;
    uint8_t  io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetchable_memory_base;
    uint16_t prefetchable_memory_limit;
    uint32_t prefetchable_base_upper_32;   /* Maybe can be unified */
    uint32_t prefetchable_limit_upper_32;
    uint16_t io_base_upper_16;
    uint16_t io_limit_upper_16;
    uint8_t  capability_pointer;  /* to be properly defined, should be a pointer ? */
    uint8_t  reserved_0;
    uint16_t reserved_1;
    uint32_t expansion_rom_base_address;
    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint16_t bridge_control;
} pci_to_pci_bridge_config_space_t ; /* should it be aligned ? */

/* PCI-to-CardBus bridge */
typedef struct {
    /* TODO To be defined */
} pci_to_cardbus_bridge_config_space_t ; /* should it be aligned ? */


/* PCI device struct */
typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_dev_t;

void pci_bus_scan(uint8_t bus);
void pci_device_scan(uint8_t bus, uint8_t device);
void pci_function_scan(uint8_t bus, uint8_t device, uint8_t function);


/* Do a recursive scan on the PCI tree starting from the root */
void pci_recursive_scan(void);

/* UGLY */
uint32_t pci_ahci_get_abar(void);


#endif // PCI_H