#include <stdio.h>
#include <libdriver.h>

/* create drvmgr_drivers[] with GPIO driver present */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_INIT
#include <rtems/confdefs.h>

int main(int arg, char **argv) 
{
	printf("GPIO test\n");
	printf("+drvmgr_init()\n");
	libdriver_init();
	drvmgr_print_topo();
	drvmgr_print_devs(PRINT_DEVS_ASSIGNED|PRINT_DEVS_UNASSIGNED|PRINT_DEVS_FAILED|PRINT_DEVS_IGNORED);
	
	printf("done\n");
	return 0;
}
