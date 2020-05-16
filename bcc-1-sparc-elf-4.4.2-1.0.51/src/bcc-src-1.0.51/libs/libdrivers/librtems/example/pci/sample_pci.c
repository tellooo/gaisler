
#include <pci.h>
#include <pci/cfg.h>

/* Select auto configuration library */
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

#include "config_pci.h"

/* Driver provide access functions */
extern struct pci_auto_setup sample_grpci2_cfg;
extern struct pci_access_drv sample_grpci2_access;

/* Change this to your board's PCI device numbers */
#define VENDOR 0x11
#define DEVICE 0x12

int main(void)
{
	struct pci_dev *pdev;
	pci_dev_t pcidev;

	/* Register PCI Driver Access library config */
	pci_access_drv_register(&sample_grpci2_access);

	/* Register PCI Driver Auto-Configuration library config */
	pci_config_register(&sample_grpci2_cfg);

	/* Do PCI statup initialization (scanning) */
	pci_config_init();

	/* Access PCI Bus */
	if (pci_find_dev(VENDOR, DEVICE, 0, &pdev) == 0) {
		pcidev = pdev->busdevfun;
		printf("Found PCI device %x:%x:%x\n", PCI_DEV_EXPAND(pcidev));
	} else {
		printf("No such PCI board found\n");
	}

	return 0;
}
