#include <rtems.h>
#include <stdio.h>
#include <errno.h>
#include <drvmgr/drvmgr.h>
#include <drvmgr/ambapp_bus_grlib.h>
#include <ambapp.h>
#include <asm-leon/leonbare_kernel.h>
#include <rtems/libio.h>

#define LEON3_IO_AREA 0xfff00000

struct ambapp_bus ambapp_plb;

/* Driver resources configuration for AMBA root bus. It is declared weak
 * so that the user may override it, if the defualt settings are not
 * enough.
 */
struct drvmgr_bus_res1 grlib_drv_resources[] __attribute__((weak)) =
{
	0, RES_EMPTY
};

/* GRLIB AMBA bus configuration (the LEON3 root bus configuration) */
struct grlib_config grlib_bus_config = 
{
  &ambapp_plb,              /* AMBAPP bus setup */
  (struct drvmgr_bus_res *)&grlib_drv_resources[0],  /* Driver configuration */
};

extern int Device_drivers_cnt;
extern rtems_driver_address_table Device_drivers[];

int do_rchar_read(struct file *f, void *b, size_t l) 
{
	rtems_libio_rw_args_t a;
	int major = major(f->node->ch_dev_id);
	int minor = minor(f->node->ch_dev_id);
	if (major >= Device_drivers_cnt)
		return ENOSYS;
	a.buffer = b;
	a.count = l;
	return Device_drivers[major].read_entry(major, minor, (void *)&a);
}

int do_rchar_write(struct file *f, void *b, size_t l)
{
	rtems_libio_rw_args_t a;
	int major = major(f->node->ch_dev_id);
	int minor = minor(f->node->ch_dev_id);
	if (major >= Device_drivers_cnt)
		return ENOSYS;

	printf("do_rchar_write\n");
	
	a.buffer = b;
	a.count = l;
	return Device_drivers[major].write_entry(major, minor, (void *)&a);
}
	
int do_rchar_ioctl(struct file *f, int r, void *arg)
{
	rtems_libio_ioctl_args_t a;
	int major = major(f->node->ch_dev_id);
	int minor = minor(f->node->ch_dev_id);
	if (major >= Device_drivers_cnt)
		return ENOSYS;
	a.command = r;
	a.buffer = arg;
	return Device_drivers[major].control_entry(major, minor, (void *)&a);
}
	
int do_rchar_open(struct file *f)
{
	rtems_libio_open_close_args_t a;
	int major = major(f->node->ch_dev_id);
	int minor = minor(f->node->ch_dev_id);
	if (major >= Device_drivers_cnt)
		return ENOSYS;
	a.flags = f->access_mode;
	a.mode = 0;
	return Device_drivers[major].open_entry(major, minor, (void *)&a);
}

int do_rchar_close(struct file *f)
{
	rtems_libio_open_close_args_t a;
	int major = major(f->node->ch_dev_id);
	int minor = minor(f->node->ch_dev_id);
	if (major >= Device_drivers_cnt)
		return ENOSYS;
	a.flags = f->access_mode;
	a.mode = 0;
	return Device_drivers[major].close_entry(major, minor, (void *)&a);
}

void libdriver_init() 
{
	int level, i;
	leonbare_base_init();

	/* plug into the io layer */
	rchar_read_func = do_rchar_read;
	rchar_write_func = do_rchar_write;
	rchar_ioctl_func = do_rchar_ioctl;
	rchar_open_func = do_rchar_open;
	rchar_close_func = do_rchar_close;
	
	ambapp_scan(&ambapp_plb, LEON3_IO_AREA, NULL, NULL);
	drv_mgr_grlib_init(&grlib_bus_config);
	_DRV_Manager_initialization();

	_DRV_Manager_init_level(1);
	
	for (i = 0; i < Device_drivers_cnt; i++) {
		Device_drivers[i].initialization_entry(i,0,0);
	}
	
	for (level=2; level<=DRVMGR_LEVEL_MAX; level++) {
		_DRV_Manager_init_level(level);
	}
}


