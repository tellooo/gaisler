#include <drvmgr/leon2_amba_bus.h>
#include <drvmgr/ambapp_bus.h>

struct drvmgr_key leon2_custom_ambapp[] =
{
	{"REG0", DRVMGR_KT_INT, {0xfff00000}}, /* IOAREA of AMBA PnP Bus */
	{"IRQ0", DRVMGR_KT_INT, {0}},
	{"IRQ1", DRVMGR_KT_INT, {1}},
	{"IRQ2", DRVMGR_KT_INT, {2}},
	{"IRQ3", DRVMGR_KT_INT, {3}},
	{"IRQ4", DRVMGR_KT_INT, {4}},
	{"IRQ5", DRVMGR_KT_INT, {5}},
	{"IRQ6", DRVMGR_KT_INT, {6}},
	{"IRQ7", DRVMGR_KT_INT, {7}},
	{"IRQ8", DRVMGR_KT_INT, {8}},
	{"IRQ9", DRVMGR_KT_INT, {9}},
	{"IRQ10", DRVMGR_KT_INT, {10}},
	{"IRQ11", DRVMGR_KT_INT, {11}},
	{"IRQ12", DRVMGR_KT_INT, {12}},
	{"IRQ13", DRVMGR_KT_INT, {13}},
	{"IRQ14", DRVMGR_KT_INT, {14}},
	{"IRQ15", DRVMGR_KT_INT, {15}},
	DRVMGR_KEY_EMPTY
};

struct leon2_core leon2_amba_custom_cores[] =
{
	/* GRLIB-LEON2 has a AMBA Plug & Play bus */
	{{LEON2_AMBA_AMBAPP_ID}, "AMBAPP", &leon2_custom_ambapp[0]},
	EMPTY_LEON2_CORE
};

struct leon2_bus leon2_bus_config =
{
	&leon2_std_cores[0],			/* The standard cores */
	&leon2_amba_custom_cores[0],		/* custom cores */
	DRVMGR_TRANSLATE_ONE2ONE,
	DRVMGR_TRANSLATE_ONE2ONE,
};

/* AMBA Resources
 */
struct drvmgr_drv_res ambapp_drv_resources[] =
{
	DRVMGR_RES_EMPTY
};

/* AMBAPP resources */
struct drvmgr_key leon2_amba_ambapp0_res[] =
{
/* {"freq", DRVMGR_KT_INTEGER, {(unsigned int)40000000}}, force 40MHz on AMBA-PnP bus */
	{"drvRes", DRVMGR_KT_POINTER, {(unsigned int)&ambapp_drv_resources}},
	DRVMGR_KEY_EMPTY
};

/* Driver resources on LEON2 AMBA bus */
struct drvmgr_bus_res leon2_amba_res =
{
	.next = NULL,
	.resource = {
		{DRIVER_LEON2_AMBA_AMBAPP, 0, &leon2_amba_ambapp0_res[0]},
		DRVMGR_RES_EMPTY
	},
};

/* GRLIB-LEON2 specific system init */
void system_init2(void)
{
#ifndef RTEMS_DRVMGR_STARTUP
	/* LEON2 PnP bus on top of standard LEON2 Bus 
	 * Note that this is only for GRLIB-LEON2 systems.
	 *
	 * Register LEON2 root bus
	 */
	leon2_root_register(&leon2_bus_config, &leon2_amba_res);
#endif
}
