
#include <drvmgr/ambapp_bus_grlib.h>
#include <drvmgr/ambapp_bus.h>
#include <ambapp_ids.h>

/* B1553RT driver configuration (optional) */
struct drvmgr_key grlib_drv_res_b1553rt0[] =
{
#ifdef SET_B1553RT_FREQ
	{"coreFreq", DRVMGR_KT_INT, {(unsigned int)SET_B1553RT_FREQ}},
#endif
	DRVMGR_KEY_EMPTY
};

/* GRPCI driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grpci0[] =
{
/*	{"byteTwisting", DRVMGR_KT_INT, {(unsigned int)1}}, */
	DRVMGR_KEY_EMPTY
};

/* GRGPIO0 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grgpio0[] =
{
	{"nBits", DRVMGR_KT_INT, {(unsigned int)24}},
	DRVMGR_KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_grgpio1[] =
{
	{"nBits", DRVMGR_KT_INT, {(unsigned int)8}},
	DRVMGR_KEY_EMPTY
};

/* GRGPIO1 driver configuration (optional) */
struct drvmgr_key grlib_drv_res_spictrl0[] =
{
#ifdef SPICTRL_SLV_SEL_FUNC
	{"slvSelFunc", DRVMGR_KT_POINTER, {(unsigned int)SPICTRL_SLV_SEL_FUNC}},
#endif
	DRVMGR_KEY_EMPTY
};

/* If RTEMS_DRVMGR_STARTUP is defined we override the "weak defaults" that
 * is defined by the LEON3 BSP.
 */
struct drvmgr_bus_res grlib_drv_resources =
{
	.next = NULL,
	.resource = {
	{DRIVER_AMBAPP_GAISLER_B1553RT_ID, 0, &grlib_drv_res_b1553rt0[0]},
	{DRIVER_AMBAPP_GAISLER_GRPCI_ID, 0, &grlib_drv_res_grpci0[0]},
	{DRIVER_AMBAPP_GAISLER_SPICTRL_ID, 0, &grlib_drv_res_spictrl0[0]},
	DRVMGR_RES_EMPTY
	}
};

#ifndef RTEMS_DRVMGR_STARTUP
struct grlib_config grlib_bus_config = 
{
	&ambapp_plb,		/* AMBAPP bus setup */
	&grlib_drv_resources,	/* Driver configuration */
};
#endif

/* LEON3 specific system init */
void system_init2(void)
{
#ifndef RTEMS_DRVMGR_STARTUP
	/* Register GRLIB root bus */
	ambapp_grlib_root_register(&grlib_bus_config);
#endif
}
