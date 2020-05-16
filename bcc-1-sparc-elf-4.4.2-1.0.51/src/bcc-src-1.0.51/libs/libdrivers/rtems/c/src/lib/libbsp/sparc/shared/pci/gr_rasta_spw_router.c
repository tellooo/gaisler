/*  GR-RASTA-IO PCI Target driver.
 * 
 *  COPYRIGHT (c) 2008.
 *  Aeroflex Gaisler AB.
 *
 *  Configures the GR-RASTA-SPW-ROUTER interface PCI board.
 *  This driver provides a AMBA PnP bus by using the general part
 *  of the AMBA PnP bus driver (ambapp_bus.c). Based on the 
 *  GR-RASTA-IO driver.
 *
 *  Driver resources for the AMBA PnP bus provided can be set using
 *  gr_rasta_io_set_resources().
 *
 *  The license and distribution terms for this file may be
 *  found in found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  2011-06-07, Marko Isomaki <marko@gaisler.com>
 *   Created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bsp.h>
#include <rtems/bspIo.h>
#include <pci.h>

#include <ambapp.h>
#include <grlib.h>
#include <drvmgr/drvmgr.h>
#include <drvmgr/ambapp_bus.h>
#include <drvmgr/pci_bus.h>
#include <genirq.h>

/*#include <gr_rasta_spw_router.h> */

/* Offset from 0x80000000 (dual bus version) */
#define AHB1_BASE_ADDR 0x80000000
#define AHB1_IOAREA_BASE_ADDR 0x80100000

#define GRPCI2_BAR0_TO_AHB_MAP 0x04  /* Fixme */
#define GRPCI2_BAR1_TO_AHB_MAP 0x08  /* Fixme */
#define GRPCI2_BAR2_TO_AHB_MAP 0x0C  /* Fixme */
#define GRPCI2_PCI_CONFIG      0x20  /* Fixme */
#define RASTA_SPW_ROUTER_OPTIONS_AMBA  0x01 /* Print AMBA bus devices */  /* Fixme */
#define RASTA_SPW_ROUTER_OPTIONS_IRQ   0x02 /* Print current IRQ setup */ /* Fixme */


/* #define DEBUG 1 */

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...) 
#endif

/* PCI ID */
#define PCIID_VENDOR_GAISLER		0x1AC8

extern int _RAM_START;

int gr_rasta_spw_router_init1(struct rtems_drvmgr_dev_info *dev);
int gr_rasta_spw_router_init2(struct rtems_drvmgr_dev_info *dev);

struct grpci2_regs {
	volatile unsigned int ctrl;
	volatile unsigned int statcap;
	volatile unsigned int pcimstprefetch;
	volatile unsigned int ahbtopciiomap;
	volatile unsigned int dmactrl;
	volatile unsigned int dmadesc;
	volatile unsigned int dmachanact;
        volatile unsigned int reserved;
	volatile unsigned int pcibartoahb[6];
	volatile unsigned int reserved2[2];
	volatile unsigned int ahbtopcimemmap[16];
	volatile unsigned int trcctrl;
	volatile unsigned int trccntmode;
	volatile unsigned int trcadpat;
        volatile unsigned int trcadmask;
	volatile unsigned int trcctrlsigpat;
	volatile unsigned int trcctrlsigmask;
        volatile unsigned int trcadstate;
	volatile unsigned int trcctrlsigstate;
};

struct gr_rasta_spw_router_ver {
	const unsigned int	amba_freq_hz;	/* The frequency */
	const unsigned int	amba_ioarea;	/* The address where the PnP IOAREA starts at */
};

/* Private data structure for driver */
struct gr_rasta_spw_router_priv {
	/* Driver management */
	struct rtems_drvmgr_dev_info	*dev;
	char				prefix[24];

	/* PCI */
	unsigned int			bar0;
	unsigned int			bar1;
        unsigned int			bar2;

	/* IRQ */
	unsigned char			irqno;            /* GR-RASTA-SPW-ROUTER System IRQ */
	genirq_t			genirq;

	/* GR-RASTA-SPW-ROUTER */
	struct gr_rasta_spw_router_ver	*version;
	LEON3_IrqCtrl_Regs_Map		*irq;
	struct grpci2_regs		*grpci2;
	struct rtems_drvmgr_mmap_entry	bus_maps[4];

	/* AMBA Plug&Play information on GR-RASTA-SPW-ROUTER */
	struct ambapp_bus		abus;
	struct ambapp_mmap		amba_maps[4];
        struct ambapp_config		config;
};

struct gr_rasta_spw_router_ver gr_rasta_spw_router_ver0 = {
	.amba_freq_hz		= 50000000,
	.amba_ioarea		= 0xfff00000,
};

int ambapp_rasta_spw_router_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg);
int ambapp_rasta_spw_router_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg);
int ambapp_rasta_spw_router_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_rasta_spw_router_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_rasta_spw_router_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg);
int ambapp_rasta_spw_router_get_params(
	struct rtems_drvmgr_dev_info *dev,
	struct rtems_drvmgr_bus_params *params);

struct ambapp_ops ambapp_rasta_spw_router_ops = {
	.int_register = ambapp_rasta_spw_router_int_register,
	.int_unregister = ambapp_rasta_spw_router_int_unregister,
	.int_enable = ambapp_rasta_spw_router_int_enable,
	.int_disable = ambapp_rasta_spw_router_int_disable,
	.int_clear = ambapp_rasta_spw_router_int_clear,
	.get_params = ambapp_rasta_spw_router_get_params
};

struct rtems_drvmgr_drv_ops gr_rasta_spw_router_ops = 
{
	.init = {gr_rasta_spw_router_init1, gr_rasta_spw_router_init2, NULL, NULL},
	.remove = NULL,
	.info = NULL
};

struct pci_dev_id gr_rasta_spw_router_ids[] = 
{
	{PCIID_VENDOR_GAISLER, PCIID_DEVICE_GR_RASTA_SPW_RTR}, 
        {0, 0}		/* Mark end of table */
};

struct pci_drv_info gr_rasta_spw_router_info =
{
	{
		NULL,				/* Next driver */
		NULL,				/* Device list */
		DRIVER_PCI_GAISLER_RASTA_SPW_ROUTER_ID,	/* Driver ID */
		"GR-RASTA-SPW_ROUTER_DRV",		/* Driver Name */
		DRVMGR_BUS_TYPE_PCI,		/* Bus Type */
		&gr_rasta_spw_router_ops,
		0,				/* No devices yet */
	},
	&gr_rasta_spw_router_ids[0]
};

/* Driver resources configuration for the AMBA bus on the GR-RASTA-SPW-ROUTER board.
 * It is declared weak so that the user may override it from the project file,
 * if the default settings are not enough.
 *
 * The configuration consists of an array of configuration pointers, each
 * pointer determine the configuration of one GR-RASTA-SPW-ROUTER board. Pointer
 * zero is for board0, pointer 1 for board1 and so on.
 *
 * The array must end with a NULL pointer.
 */
struct rtems_drvmgr_drv_res *gr_rasta_spw_router_resources[] __attribute__((weak)) =
{
	NULL
};
int gr_rasta_spw_router_resources_cnt = 0;

void gr_rasta_spw_router_register_drv(void)
{
	DBG("Registering GR-RASTA-SPW-ROUTER PCI driver\n");
	rtems_drvmgr_drv_register(&gr_rasta_spw_router_info.general);
}

void gr_rasta_spw_router_isr (int irqno, struct gr_rasta_spw_router_priv *priv)
{
	unsigned int status, tmp;
	int irq;
	tmp = status = priv->irq->ipend;

	/* DBG("GR-RASTA-SPW-ROUTER: IRQ 0x%x\n",status); */
        
        for(irq=0; irq<16; irq++) {
		if ( status & (1<<irq) ) {
			genirq_doirq(priv->genirq, irq);
			priv->irq->iclear = (1<<irq);
			status &= ~(1<<irq);
			if ( status == 0 )
				break;
		}
	}

	/* ACK interrupt, this is because PCI is Level, so the IRQ Controller still drives the IRQ. */
	if ( tmp ) 
		rtems_drvmgr_interrupt_clear(priv->dev, 0, gr_rasta_spw_router_isr, (void *)priv);

	DBG("RASTA-SPW_ROUTER-IRQ: 0x%x\n", tmp);
}

/* AMBA PP find routines */
int gr_rasta_spw_router_dev_find(struct ambapp_dev *dev, int index, int maxdepth, void *arg)
{
	/* Found IRQ/GRPCI2 controller, stop */
	*(struct ambapp_dev **)arg = dev;
	return 1;
}

int gr_rasta_spw_router_hw_init(struct gr_rasta_spw_router_priv *priv)
{
        int i;
        unsigned int data;
        uint8_t tmp2;
        int bus, dev, fun;
	struct pci_dev_info *devinfo;
	unsigned char ver;
	struct ambapp_dev *tmp;
	int status;
	struct ambapp_ahb_info *ahb;
        uint8_t cap_ptr;
        
        devinfo = (struct pci_dev_info *)priv->dev->businfo;

	bus = devinfo->bus;
	dev = devinfo->dev;
	fun = devinfo->func;

	pci_read_config_dword(bus, dev, fun, PCI_BASE_ADDRESS_0, &priv->bar0);
	pci_read_config_byte(bus, dev, fun, PCI_INTERRUPT_LINE, &priv->irqno);

        if ( priv->bar0 == 0 ) {
		/* Not all neccessary space assigned to GR-RASTA-SPW-ROUTER target */
		return -1;
	}

	/* Select version of GR-RASTA-SPW-ROUTER board. Currently only one version */
	pci_read_config_byte(bus, dev, fun, PCI_REVISION_ID, &ver);
	switch (ver) {
		case 0:
			priv->version = &gr_rasta_spw_router_ver0;
			break;
                default:
			return -2;
	}

        /* Check capabilities list bit */
        pci_read_config_byte(bus, dev, fun, PCI_STATUS, &tmp2);

        if (!((tmp2 >> 4) & 1)) {
                /* Capabilities list not available which it should be in the GRPCI2 */
                return -3;
        }
        
        /* Read capabilities pointer */
        pci_read_config_byte(bus, dev, fun, PCI_CAP_PTR, &cap_ptr);

        /* Set AHB address mappings for target PCI bars */
        pci_write_config_dword(bus, dev, fun, cap_ptr+GRPCI2_BAR0_TO_AHB_MAP, 0xffe00000);  /* APB bus, AHB I/O bus 2 MB */ 
        
        printf(" PCI BAR[0]: 0x%x, IRQ: %d\n\n\n", priv->bar0, priv->irqno);

        /* Set PCI bus to be big endian */
        pci_read_config_dword(bus, dev, fun, cap_ptr+GRPCI2_PCI_CONFIG, &data);
        data = data & 0xFFFFFFFE;
        pci_write_config_dword(bus, dev, fun, cap_ptr+GRPCI2_PCI_CONFIG, data);
        
#if 0
	/* set parity error response */
	pci_read_config_dword(bus, dev, fun, PCI_COMMAND, &data);
	pci_write_config_dword(bus, dev, fun, PCI_COMMAND, (data|PCI_COMMAND_PARITY));
#endif

	/* Scan AMBA Plug&Play */

	/* AMBA MAP bar0 (in router) ==> 0xffe00000(remote amba address) */
	priv->amba_maps[0].size = 0x00200000;
	priv->amba_maps[0].local_adr = priv->bar0;
	priv->amba_maps[0].remote_adr = 0xffe00000;

	/* Mark end of table */
	priv->amba_maps[1].size=0;
	priv->amba_maps[1].local_adr = 0;
	priv->amba_maps[1].remote_adr = 0;

	/* Start AMBA PnP scan at first AHB bus */
	ambapp_scan(&priv->abus, priv->bar0+0x100000, NULL, &priv->amba_maps);

	/* Initialize Frequency of AMBA bus */
	ambapp_freq_init(&priv->abus, NULL, priv->version->amba_freq_hz);

	/* Find IRQ controller, Clear all current IRQs */
	tmp = NULL;
	status = ambapp_for_each(priv->abus.root, (OPTIONS_ALL|OPTIONS_APB_SLVS), VENDOR_GAISLER, GAISLER_IRQMP, 10, gr_rasta_spw_router_dev_find, &tmp);
	if ( (status != 1) || !tmp ) {
		return -4;
	}
	priv->irq = (LEON3_IrqCtrl_Regs_Map *)(((struct ambapp_apb_info *)tmp->devinfo)->start);
	/* Set up GR-RASTA-SPW-ROUTER irq controller */
	priv->irq->mask[0] = 0;
	priv->irq->iclear = 0xffff;
	priv->irq->ilevel = 0;

	priv->bus_maps[0].map_size = priv->amba_maps[0].size;
	priv->bus_maps[0].local_adr = priv->amba_maps[0].local_adr;
	priv->bus_maps[0].remote_adr = priv->amba_maps[0].remote_adr;

	/* Find GRPCI2 controller AHB Slave interface */
	tmp = NULL;
	status = ambapp_for_each(priv->abus.root, (OPTIONS_ALL|OPTIONS_AHB_SLVS), VENDOR_GAISLER, GAISLER_GRPCI2, 10, gr_rasta_spw_router_dev_find, &tmp);
	if ( (status != 1) || !tmp ) {
		return -5;
	}
	ahb = (struct ambapp_ahb_info *)tmp->devinfo;
	priv->bus_maps[1].map_size = ahb->mask[0]; /* AMBA->PCI Window on GR-RASTA-IO board */
	priv->bus_maps[1].local_adr = &_RAM_START;
	priv->bus_maps[1].remote_adr = ahb->start[0];
        
        priv->bus_maps[2].map_size = 0;
	priv->bus_maps[2].local_adr = 0;
	priv->bus_maps[2].remote_adr = 0;

        /* Find GRPCI2 controller APB Slave interface */
        tmp = NULL;
	status = ambapp_for_each(priv->abus.root, (OPTIONS_ALL|OPTIONS_APB_SLVS), VENDOR_GAISLER, GAISLER_GRPCI2, 10, gr_rasta_spw_router_dev_find, &tmp);
	if ( (status != 1) || !tmp ) {
		return -6;
	}
	priv->grpci2 = (struct grpci_regs *)((struct ambapp_apb_info *)tmp->devinfo)->start;
        
        /* Set AHB to PCI mapping for all masters*/
        for(i = 0; i < 16; i++) {
                priv->grpci2->ahbtopcimemmap[i] = &_RAM_START;
        }
        /* Make sure dirq(0) sampling is enabled */
        data = priv->grpci2->ctrl;
        data = (data & 0xFFFFFF0F) | (1 << 4);
        printf("data: %x\n", data);
        priv->grpci2->ctrl = data;
        
        /* Successfully registered the RASTA-SPW-ROUTER board */
	return 0;
}

void gr_rasta_spw_router_hw_init2(struct gr_rasta_spw_router_priv *priv)
{
	int bus, dev, fun;
	struct pci_dev_info *devinfo;

	devinfo = (struct pci_dev_info *)priv->dev->businfo;

	bus = devinfo->bus;
	dev = devinfo->dev;
	fun = devinfo->func;

	/* Enable DMA by enabling PCI target as master */
	pci_master_enable(bus, dev, fun);
}

/* Called when a PCI target is found with the PCI device and vendor ID 
 * given in gr_rasta_spw_router_ids[].
 */
int gr_rasta_spw_router_init1(struct rtems_drvmgr_dev_info *dev)
{
	struct gr_rasta_spw_router_priv *priv;
	struct pci_dev_info *devinfo;
	int status;

	priv = malloc(sizeof(struct gr_rasta_spw_router_priv));
	if ( !priv )
		return DRVMGR_NOMEM;

	memset(priv, 0, sizeof(*priv));
	dev->priv = priv;
	priv->dev = dev;

	/* Determine number of configurations */
	if ( gr_rasta_spw_router_resources_cnt == 0 ) {
		while ( gr_rasta_spw_router_resources[gr_rasta_spw_router_resources_cnt] )
			gr_rasta_spw_router_resources_cnt++;
	}

	/* Generate Device prefix */

	strcpy(priv->prefix, "/dev/rastaspwrouter0");
	priv->prefix[19] += dev->minor_drv;
	mkdir(priv->prefix, S_IRWXU | S_IRWXG | S_IRWXO);
	priv->prefix[20] = '/';
	priv->prefix[21] = '\0';

	devinfo = (struct pci_dev_info *)dev->businfo;
	printf("\n\n--- GR-RASTA-SPW-ROUTER[%d] ---\n", dev->minor_drv);
	printf(" PCI BUS: %d, SLOT: %d, FUNCTION: %d\n", devinfo->bus, devinfo->dev, devinfo->func);
	printf(" PCI VENDOR: 0x%04x, DEVICE: 0x%04x\n", devinfo->id.vendor, devinfo->id.device);

	priv->genirq = genirq_init(16);
	if ( priv->genirq == NULL ) {
		free(priv);
		dev->priv = NULL;
		return DRVMGR_FAIL;
	}

	if ( status = gr_rasta_spw_router_hw_init(priv) ) {
		genirq_destroy(priv->genirq);
		free(priv);
		dev->priv = NULL;
		printf(" Failed to initialize GR-RASTA-SPW-ROUTER HW: %d\n", status);
		return DRVMGR_FAIL;
	}

	/* Init amba bus */
	priv->config.abus = &priv->abus;
	priv->config.ops = &ambapp_rasta_spw_router_ops;
	priv->config.mmaps = &priv->bus_maps[0];
	if ( priv->dev->minor_drv < gr_rasta_spw_router_resources_cnt ) {
		priv->config.resources = gr_rasta_spw_router_resources[priv->dev->minor_drv];
	} else {
		priv->config.resources = NULL;
	}

	/* Create and register AMBA PnP bus. */
	return ambapp_bus_register(dev, &priv->config);
}

int gr_rasta_spw_router_init2(struct rtems_drvmgr_dev_info *dev)
{
	struct gr_rasta_spw_router_priv *priv = dev->priv;

	/* install interrupt vector */
	rtems_drvmgr_interrupt_register(priv->dev, 0,
		gr_rasta_spw_router_isr, (void *)priv);

	/* Clear any old interrupt requests */
	rtems_drvmgr_interrupt_clear(priv->dev, 0,
		gr_rasta_spw_router_isr, (void *)priv);

	/* Enable System IRQ so that GR-RASTA-SPW_ROUTER PCI target interrupt goes
	 * through.
	 *
	 * It is important to enable it in stage init2. If interrupts were
	 * enabled in init1 this might hang the system when more than one
	 * PCI board is connected, this is because PCI interrupts might
	 * be shared and PCI board 2 have not initialized and
	 * might therefore drive interrupt already when entering init1().
	 */
	rtems_drvmgr_interrupt_enable(dev, 0, gr_rasta_spw_router_isr, (void *)priv);

	gr_rasta_spw_router_hw_init2(priv);

	return DRVMGR_OK;
}

int ambapp_rasta_spw_router_int_register(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr handler,
	void *arg)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	rtems_interrupt_disable(level);
        
        printf("Irq: %d\n", irq);
	status = genirq_register(priv->genirq, irq, handler, arg);
	if ( status == 0 ) {
		/* Disable and clear IRQ for first registered handler */
		priv->irq->iclear = (1<<irq);
		priv->irq->mask[0] &= ~(1<<irq); /* mask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_rasta_spw_router_int_unregister(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	rtems_interrupt_disable(level);

	status = genirq_unregister(priv->genirq, irq, isr, arg);
	if ( status != 0 )
		status = -1;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_rasta_spw_router_int_enable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	DBG("RASTA-SPW-ROUTER IRQ %d: enable\n", irq);

	rtems_interrupt_disable(level);

	status = genirq_enable(priv->genirq, irq, isr, arg);
	if ( status == 0 ) {
		/* Enable IRQ for first enabled handler only */
		priv->irq->mask[0] |= (1<<irq); /* unmask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_rasta_spw_router_int_disable(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;
	rtems_interrupt_level level;
	int status;

	DBG("RASTA-IO IRQ %d: disable\n", irq);

	rtems_interrupt_disable(level);

	status = genirq_disable(priv->genirq, irq, isr, arg);
	if ( status == 0 ) {
		/* Disable IRQ only when no enabled handler exists */
		priv->irq->mask[0] &= ~(1<<irq); /* mask interrupt source */
	} else if ( status == 1 )
		status = 0;

	rtems_interrupt_enable(level);

	return status;
}

int ambapp_rasta_spw_router_int_clear(
	struct rtems_drvmgr_dev_info *dev,
	int irq,
	rtems_drvmgr_isr isr,
	void *arg)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;

	if ( genirq_check(priv->genirq, irq) )
		return -1;

	priv->irq->iclear = (1<<irq);

	return 0;
}

int ambapp_rasta_spw_router_get_params(struct rtems_drvmgr_dev_info *dev, struct rtems_drvmgr_bus_params *params)
{
	struct gr_rasta_spw_router_priv *priv = dev->parent->dev->priv;

	/* Device name prefix pointer, skip /dev */
	params->dev_prefix = &priv->prefix[5];

	return 0;
}

void gr_rasta_spw_router_print_dev(struct rtems_drvmgr_dev_info *dev, int options)
{
	struct gr_rasta_spw_router_priv *priv = dev->priv;
	struct pci_dev_info *devinfo;
	int bus, device, fun;
	
        devinfo = (struct pci_dev_info *)priv->dev->businfo;

	bus = devinfo->bus;
	device = devinfo->dev;
	fun = devinfo->func;

	/* Print */
	printf("--- GR-RASTA-SPW-ROUTER [bus %d, dev %d, fun %d] ---\n", bus, device, fun);
	printf(" PCI BAR0:        0x%x\n", priv->bar0);
	printf(" PCI BAR1:        0x%x\n", priv->bar1);
        printf(" PCI BAR2:        0x%x\n", priv->bar2);
	printf(" IRQ REGS:        0x%x\n", (unsigned int)priv->irq);
	printf(" IRQ:             %d\n", priv->irqno);
	printf(" FREQ:            %d Hz\n", priv->version->amba_freq_hz);
	printf(" IMASK:           0x%08x\n", priv->irq->mask[0]);
	printf(" IPEND:           0x%08x\n", priv->irq->ipend);

	/* Print amba config */
	if ( options & RASTA_SPW_ROUTER_OPTIONS_AMBA ) {
		ambapp_print(&priv->abus, 10);
	}

#if 0
	/* Print IRQ handlers and their arguments */
	if ( options & RASTA_SPW_ROUTER_OPTIONS_IRQ ) {
		for(i=0; i<16; i++) {
			printf(" IRQ[%02d]:         0x%x, arg: 0x%x\n", 
				i, (unsigned int)priv->isrs[i].handler, (unsigned int)priv->isrs[i].arg);
		}
	}
#endif
}

void gr_rasta_spw_router_print(int options)
{
	struct pci_drv_info *drv = &gr_rasta_spw_router_info;
	struct rtems_drvmgr_dev_info *dev;

	dev = drv->general.dev;
	while(dev) {
		gr_rasta_spw_router_print_dev(dev, options);
		dev = dev->next_in_drv;
	}
}
