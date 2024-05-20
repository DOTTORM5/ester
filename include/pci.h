#ifndef PCI_H
#define PCI_H

#include "types.h"

/* I/O port address for PCI configuration space in classic mode mechanism 1 */
#define PCI_CONFIG_ADDRESS_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT    0xCFC


/* Read a long word (32 bit) from a PCI configuration space of the given device */
static __u32 pci_read_config_long(__u8 bus, __u8 device, __u8 function, __u8 offset);

#endif // PCI_H