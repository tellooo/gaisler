#include <stdio.h>
#include <libdriver.h>
#include <asm-leon/leon.h>

/* create drvmgr_drivers[] with uart driver present */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_INIT
#include <rtems/confdefs.h>

int main(int arg, char **argv) 
{
	FILE *f; int fd;
	
	printf("UART test\n");

	outbyte_enter_f = outbyte_enter;
	outbyte_exit_f = outbyte_exit;
	
	libdriver_init();
	drvmgr_print_topo();
	drvmgr_print_devs(PRINT_DEVS_ASSIGNED|PRINT_DEVS_UNASSIGNED|PRINT_DEVS_FAILED|PRINT_DEVS_IGNORED);

	printf("Try open /dev/console\n");
	
	if ((f = fopen("/dev/console","w"))) {
		fprintf(f, "Hello\n");
		fclose(f);
	}
	
	printf("done\n");
	return 0;
}
