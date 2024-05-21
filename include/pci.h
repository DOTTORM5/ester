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
    __u16 vendor_id;
    __u16 device_id;
    __u16 command;
    __u16 status;
    __u8  revision_id;
    __u8  prog_if;
    __u8  subclass_code;
    __u8  class_code;
    __u8  cache_line_size;
    __u8  latency_timer;
    __u8  header_type;   /* this is always 0x0 for a general device */
    __u8  bist;
    __u32 bar_0;
    __u32 bar_1;
    __u32 bar_2;
    __u32 bar_3;
    __u32 bar_4;
    __u32 bar_5;
    __u32 cardbus_cis_pointer;  /* to be properly defined, should be a pointer ? */
    __u16 subsystem_vendor_id;
    __u16 subsystem_id;
    __u32 expansion_rom_base_address;
    __u8  capabilities_pointer;  /* to be properly defined, should be a pointer ? */
    __u8  reserved_0;
    __u16 reserved_1;
    __u32 reserved_2;
    __u8  interrupt_line;
    __u8  interrupt_pin;
    __u8  min_grant;
    __u8  max_latency;
} __general_device_config_space ; /* should it be aligned ? */

/* PCI-to-PCI bridge */
typedef struct {
    __u16 vendor_id;
    __u16 device_id;
    __u16 command;
    __u16 status;
    __u8  revision_id;
    __u8  prog_if;
    __u8  subclass_code;
    __u8  class_code;
    __u8  cache_line_size;
    __u8  latency_timer;
    __u8  header_type;   /* this is always 0x1 for a pci-to-pci bridge */
    __u8  bist;
    __u32 bar_0;
    __u32 bar_1;
    __u8  primary_bus_number;
    __u8  secondary_bus_number;
    __u8  subordinate_bus_number;
    __u8  secondary_latency_timer;
    __u8  io_base;
    __u8  io_limit;
    __u16 secondary_status;
    __u16 memory_base;
    __u16 memory_limit;
    __u16 prefetchable_memory_base;
    __u16 prefetchable_memory_limit;
    __u32 prefetchable_base_upper_32;   /* Maybe can be unified */
    __u32 prefetchable_limit_upper_32;
    __u16 io_base_upper_16;
    __u16 io_limit_upper_16;
    __u8  capability_pointer;  /* to be properly defined, should be a pointer ? */
    __u8  reserved_0;
    __u16 reserved_1;
    __u32 expansion_rom_base_address;
    __u8  interrupt_line;
    __u8  interrupt_pin;
    __u16 bridge_control;
} __pci_to_pci_bridge_config_space ; /* should it be aligned ? */

/* PCI-to-CardBus bridge */
typedef struct {
    /* To be defined */
} __pci_to_cardbus_bridge_config_space ; /* should it be aligned ? */


/* PCI device struct */
typedef struct {
    __u8 bus;
    __u8 device;
    __u8 function;
} __pci_dev;

/* This containts all pci_devices in terms of bus|dev|func, this should be a vector dinamically allocated, but we don't have heap strategy implemented yet, we wrongly give it a fixed size for now */
static struct {
    __pci_dev pci_devices[PCI_MAX_DEVICES];
    __u16 pci_devices_cnt;     /* Effective number of pci_devices */
} pci_vector_devices ;

/* PCI device struct management */

/* Initialize the pci_vector structure */
void pci_vector_dev_init(void);

/* Create a __pci_dev */
static __pci_dev pci_dev_create(__u8 bus, __u8 device, __u8 function);

/* Return the number of PCI dev actually present */
__u16 pci_vector_dev_cnt(void);

/* Return a __pci_dev present indexed by index, this is not destructive */
__pci_dev pci_vector_dev_get(__u16 index);

/* Add a device in terms of bus|device|function to the PCI vector device struct */
__u8 pci_vector_dev_add(__u8 bus, __u8 device, __u8 function);


/* UTILITY */
/* For all devices */

/* Read a long word (32 bit) from a PCI configuration space of the given device */
static __u32 pci_read_config_long(__u8 bus, __u8 device, __u8 function, __u8 offset); /* Should be inline ? */

/* Get the vendor id of a pci device */
static __u16 pci_get_vendor_id(__u8 bus, __u8 device, __u8 function); /* Should be inline ? */

/* Get the header type of a pci device */
static __u8 pci_get_header_type(__u8 bus, __u8 device, __u8 function);

/* Get the class code of a pci device */
static __u8 pci_get_class_code(__u8 bus, __u8 device, __u8 function);

/* Get the sub class code of a pci device */
static __u8 pci_get_sub_class_code(__u8 bus, __u8 device, __u8 function);


/* For bridge only */

/* Get the secondary bus attached to a PCI-to-PCI bridge, this is valid only if you call it on a valid bridge */
static __u8 pci_get_secondary_bus(__u8 bus, __u8 device, __u8 function);


/* SCANNING routines */

/* Scan a given PCI function */
static void pci_function_scan(__u8 bus, __u8 device, __u8 function);

/* Scan a given PCI device */
static void pci_device_scan(__u8 bus, __u8 device);

/* Scan a given PCI bus */
static void pci_bus_scan(__u8 bus);

/* Do a recursive scan on the PCI tree starting from the root */
void pci_recursive_scan(void);

#endif // PCI_H