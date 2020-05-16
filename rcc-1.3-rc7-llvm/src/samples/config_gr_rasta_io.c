#include <grlib/gr_rasta_io.h>
#include <grlib/ambapp_bus.h>

/* GR-RASTA-IO boards configuration example. Note that this is 
 * optional, we only override defaults. If default are ok, nothing
 * is need to be done.
 */

/*** CONFIGURES MEMORY CONTROLLER ***/
struct drvmgr_key rastaioN_mctrl0_res[] =
{
#ifdef RASTA_IO_SRAM
	/* SRAM */
#ifdef RASTA_IO_30MHZ
/* Configuration for a GR-RASTA-IO 30MHz board */
	{"mcfg1", DRVMGR_KT_INT, {(unsigned int) 0x000002ff}},
	{"mcfg2", DRVMGR_KT_INT, {(unsigned int) 0x00001260}}, /* 0x00001265 */
	{"mcfg3", DRVMGR_KT_INT, {(unsigned int) 0x000e8000}},
#else
/* Configuration for a GR-RASTA-IO 50MHz board */
	{"mcfg1", DRVMGR_KT_INT, {(unsigned int) 0x0003c2ff}},
	{"mcfg2", DRVMGR_KT_INT, {(unsigned int) 0x00001265}},
	{"mcfg3", DRVMGR_KT_INT, {(unsigned int) 0x00184000}},
#endif
#else
	/* SDRAM */
	{"mcfg1", DRVMGR_KT_INT, {(unsigned int) 0x000002ff}},
	{"mcfg2", DRVMGR_KT_INT, {(unsigned int) 0x82206000}},
	{"mcfg3", DRVMGR_KT_INT, {(unsigned int) 0x000e8000}},
#endif
	DRVMGR_KEY_EMPTY
};

/* Configures Timer prescaler ***/
struct drvmgr_key rastaioN_gptimer0_res[] =
{
/* Set prescaler so that a 10MHz clock is generated */
#ifdef RASTA_IO_30MHZ
	/* 30MHz system needs dividing with 3 */
	{"prescaler", DRVMGR_KT_INT, {(unsigned int) 30/10-1}},
#else
	/* 50MHz system needs dividing with 5 */
	{"prescaler", DRVMGR_KT_INT, {(unsigned int) 50/10-1}},
#endif
	DRVMGR_KEY_EMPTY
};

/* Configuration for a GR-RASTA-IO 1553BRM driver */
struct drvmgr_key rastaio0_b1553brm_res[] = 
{
#ifdef RASTAIO_B1553RBM_REMOTE_ADDR
	/* Use address 0x40000000 on the RASTA-IO board. This will be
	 * translated into a CPU address, probably 0xA0000000 on a AT697
	 * PCI board.
	 */
	{"dmaBaseAdr", DRVMGR_KT_POINTER, {(unsigned int) 0x40000001}}, /* 1 to indicate remote address */
#endif
	DRVMGR_KEY_EMPTY
};

/*** Driver resources for GR-RASTA-IO 0 AMBA PnP bus ***/
struct drvmgr_bus_res gr_rasta_io0_res =
{
	.next = NULL,
	.resource = {
		{DRIVER_AMBAPP_ESA_MCTRL_ID, 0, &rastaioN_mctrl0_res[0]},
		{DRIVER_AMBAPP_GAISLER_GPTIMER_ID, 0, &rastaioN_gptimer0_res[0]},
		{DRIVER_AMBAPP_GAISLER_B1553BRM_ID, 0, &rastaio0_b1553brm_res[0]},
		DRVMGR_RES_EMPTY
	},
};

/*** Driver resources for GR-RASTA-IO 1 AMBA PnP bus ***/
struct drvmgr_bus_res gr_rasta_io1_res =
{
	.next = NULL,
	.resource = {
		{DRIVER_AMBAPP_ESA_MCTRL_ID, 0, &rastaioN_mctrl0_res[0]},
		DRVMGR_RES_EMPTY
	},
};

/* Tell GR-RASTA-IO driver about the bus resources.
 * Resources for one GR-RASTA-IO board are available.
 * AMBAPP->PCI->GR-RASTA-IO->AMBAPP bus resources
 *
 * The resources will be used by the drivers for the 
 * cores found on the GR-RASTA-IO->AMBAPP bus.
 *
 * The "weak defaults" are overriden here.
 */
struct drvmgr_bus_res *gr_rasta_io_resources[] = 
{
	&gr_rasta_io0_res,		/* GR-RASTA-IO board 1 resources */
	&gr_rasta_io1_res,		/* GR-RASTA-IO board 2 resources */
	NULL,				/* End of table */
};

void system_init_rastaio(void)
{
	/* Nothing need to be done, the "weak defaults" are overriden by defining
	 * gr_rasta_io_resources[]
	 */
}
