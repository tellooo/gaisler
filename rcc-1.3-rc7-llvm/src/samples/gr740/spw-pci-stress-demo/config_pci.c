#include <drvmgr/pci_bus.h>

#ifdef CONFIGURE_DRIVER_PCI_GR_RASTA_IO
/* GR-RASTA-IO[0] PCI Peripheral driver configuration example */
struct drvmgr_key rastaio_drv_res_0[] =
{
	/* Note: this is just made up, the GR-RASTA-IO driver does not have
	 *       any configuration options at this point.
	 */
	{"noDMA", DRVMGR_KT_INT, {(unsigned int)1}},
	DRVMGR_KEY_EMPTY
};

/* GR-RASTA-IO[1] PCI Peripheral driver configuration example */
struct drvmgr_key rastaio_drv_res_1[] =
{
	{"noDMA", DRVMGR_KT_INT, {(unsigned int)0}},
	DRVMGR_KEY_EMPTY
};
#endif

/* Driver resources configuration for the PCI bus. Add optional configuration
 * options (driver resources) in the resource array below.
 *
 * Each PCI device instance may be configured.
 */
struct drvmgr_bus_res pcibus_drv_resources = {
	.next = NULL,
	.resource = {
		DRVMGR_RES_EMPTY,
#ifdef CONFIGURE_DRIVER_PCI_GR_RASTA_IO
		{DRIVER_PCI_GAISLER_RASTAIO_ID, 0, &rastaio_drv_res_0[0]},
		{DRIVER_PCI_GAISLER_RASTAIO_ID, 1, &rastaio_drv_res_1[0]},
#endif
	},
};
