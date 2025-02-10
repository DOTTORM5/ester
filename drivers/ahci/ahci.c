
#include "ahci.h"
#include "printk.h"
#include "mem.h"
#include "pci.h"

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM   	0x96690101	// Port multiplier

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define	AHCI_BASE	0x40000	// 4M

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define ATA_CMD_READ_DMA_EX  0xC8
#define ATA_CMD_WRITE_DMA_EX 0x35

#define HBA_PxIS_TFES (1 << 30)


static HBA_MEM * hba_mem_ptr;

// Stop command engine
void stop_cmd(HBA_PORT *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;

	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;

	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

// Start command engine
void start_cmd(HBA_PORT *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);

	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}


void port_rebase(HBA_PORT *port, int portno)
{
	stop_cmd(port);	// Stop command engine

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (0<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);

	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);

	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
		
		// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
	start_cmd(port);	// Start command engine
}


// Check device type
static int ahci_check_type(HBA_PORT *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

void ahci_probe_port(HBA_MEM *abar)
{
	// Search disk in implemented ports
	uint32_t pi = abar->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = ahci_check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
			{
				printk("SATA drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				printk("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				printk("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				printk("PM drive found at port %d\n", i);
			}
			else
			{
				printk("No drive found at port %d\n", i);
			}
		}

		pi >>= 1;
		i ++;
	}
}

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

// Find a free command list slot
int find_cmdslot(HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = (port->sact | port->ci);
	for (int i=0; i<32; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	printk("Cannot find free command list entry\n");
	return -1;
}

uint8_t read(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, char *buf)
{
	port->is = (uint32_t) -1;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 1;

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += 0;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;	// PRDT entries count

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	
	memset((void *) cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl)*sizeof(HBA_PRDT_ENTRY));
	
	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	for (i=0; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis =(FIS_REG_H2D*)(cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D ;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
	cmdfis->control = 0x08;

	cmdfis->lba0 = (uint8_t) startl;
	cmdfis->lba1 = (uint8_t) (startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode

	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printk("Port is hung\n");
		return 1;
	}

	port->ci = 1<<slot;	// Issue command

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			printk("Read disk error\n");
			return 1;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printk("Read disk error\n");
		return 1;
	}

	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printk("Port is hung\n");
		return 1;
	}
	return 0;
}


uint8_t write(HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, char *buf)
{
    port->is = (uint32_t) -1;  // Clear pending interrupt bits
	int spin = 0;              // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 1;

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Command FIS size
	cmdheader->w = 1;  // Set to 1 for write
	cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;  // PRDT entries count

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset((void *)cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	for (i = 0; i < cmdheader->prdtl-1; i++) {
		cmdtbl->prdt_entry[i].dba =  (uint32_t) ((uint64_t)buf & 0xffffffff);
		cmdtbl->prdt_entry[i].dbau = ( ( ((uint64_t)(buf)) >> 32) & 0xffffffff);
		cmdtbl->prdt_entry[i].dbc = 4096-1;  // 8K bytes per PRDT
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;  // 4K words
		count -= 16;  // 16 sectors
	}

	// Last entry
	cmdtbl->prdt_entry[i].dba =  (uint32_t) ((uint64_t)buf & 0xffffffff);
	cmdtbl->prdt_entry[i].dbau = ( ( ((uint64_t)(buf)) >> 32) & 0xffffffff);

	cmdtbl->prdt_entry[i].dbc = ((count * 512) % 4096) - 1;  // 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;  // Command
	cmdfis->command = ATA_CMD_WRITE_DMA_EX;  // Write command

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6;  // LBA mode

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// Wait until the port is not busy
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		spin++;
	}
	if (spin == 1000000) {
		printk("Port is hung\n");
		return 1;
	}

	port->ci = 1 << slot;  // Issue command

	// Wait for completion
	while (1) {
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES) {  // Task file error
			printk("Write disk error\n");
			return 1;
		}
	}

	// Check for errors again
	if (port->is & HBA_PxIS_TFES) {
		printk("Write disk error\n");
		return 1;
	}

	return 0;  // Success
}


uint8_t ahci_init() 
{
	uint32_t ahci_phys_addr = pci_ahci_get_abar();
	pmap(ahci_phys_addr, ahci_phys_addr, 0);
	hba_mem_ptr = (HBA_MEM *) ahci_phys_addr; 
	hba_mem_ptr->ghc |= AHCI_GHC_HR; /* Reset the controller */
	while (hba_mem_ptr->ghc & AHCI_GHC_HR); /* Wait the reset complention */
	hba_mem_ptr->ghc |= (AHCI_GHC_AE | AHCI_IE);
	ahci_probe_port(hba_mem_ptr);
	port_rebase(&hba_mem_ptr->ports[0], 0);
	return 0;
}

uint8_t ahci_write (uint8_t port_index, uint64_t lba, uint32_t count, char *buf)
{
	uint32_t startl = (uint32_t) lba & 0xFFFFFFFF;
	uint32_t starth = (uint32_t) ( (lba & 0xFFFFFFFF00000000) >> 32 );
	return write(hba_mem_ptr->ports, startl, starth, count, buf);
}

uint8_t ahci_read(uint8_t port_index, uint64_t lba, uint32_t count, char *buf) 
{
	uint32_t startl = (uint32_t) lba & 0xFFFFFFFF;
	uint32_t starth = (uint32_t) ( (lba & 0xFFFFFFFF00000000) >> 32 );
	return read(hba_mem_ptr->ports, startl, starth, count, buf);
}
	