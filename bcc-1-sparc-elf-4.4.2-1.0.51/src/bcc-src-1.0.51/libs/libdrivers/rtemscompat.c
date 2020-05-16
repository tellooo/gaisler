#include <stdio.h>
#include <rtems.h>
#include <drvmgr/drvmgr.h>
#include <asm-leon/leonbare_kernel.h>
#include <asm-leon/nosys.h>
#include <sys/stat.h>
#include <cons.h>
#include <rtems/libio.h>

extern rtems_driver_address_table Device_drivers[];
extern int Device_drivers_cnt;

/* Enable debugging */
#define DEBUG 1

#ifdef DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif

void rtems_fatal_error_occurred(int the_error ) {
	
}

char *rtems_build_name(char a0,char a1,char a2,char a3 ) 
{
	return 0;
}

/* ================ */
/* semaphore switch */
int rtems_semaphore_release(rtems_id id) {
	rtems_id i = id;
	leonbare_semaphore_release((void *)&i);
	return 0;
}

int rtems_semaphore_create(char *name, int count, int attribute_set, int priority_ceiling, rtems_id *id ) 
{
	int flags = attribute_set;
	leonbare_semaphore_create(name, count, flags, 0, (void *)id);
	return 0;
}

int rtems_semaphore_obtain(rtems_id       id,
					 int option_set,
					 int timeout ) {
	rtems_id i = id; int f0, f1;
	leonbare_semaphore_obtain((void *)&i, f0, f1);
	return 0;
}

/* ================ */
/* irq */
static int irqs_cnt = 0; 
static struct irqaction irqs[32];
int BSP_shared_interrupt_register (int irq,
				   drvmgr_isr isr,
				   void *arg)
{
	int ret, j; struct irqaction *i = 0;
	if (irqs_cnt >= 32) {
		DBG("@Register interrupt handler: out of handler structs\n");
		return -1;
	}
	DBG("@Register interrupt handler 0x%x irq %d arg 0x%x\n", isr, irq, arg);

	for (j = 0; j < irqs_cnt; j++) {
		i = &irqs[j];
		if (!i->handler)
			break;
	}
	if (i == 0 || j >= irqs_cnt)
		i = &irqs[irqs_cnt++];
	i->handler = (irqhandler) isr;
	i->dev_id = arg;
	i->flags = 0;
	chained_catch_interrupt (irq, i);
	return 0;
}

int BSP_shared_interrupt_unregister (int irq,
				     drvmgr_isr isr,
				     void *arg )
{
	int ret, j; struct irqaction *i;
	for (j = 0; j < irqs_cnt; j++) {
		i = &irqs[j];
		if (i->handler == (irqhandler) isr &&
		    i->dev_id == arg) {
			if (chained_uncatch_interrupt (irq, i))
				i->handler = 0;
		}
	}
	return 0;
}

int BSP_shared_interrupt_enable
	(
	int irq
	)
{
	int ret;
	return ret;
}

int BSP_shared_interrupt_disable
	(
	int irq
	)
{
	int ret;
	return ret;
}

void BSP_shared_interrupt_mask(int irq)
{
	
}

void BSP_shared_interrupt_unmask(int irq)
{
	
}

int BSP_shared_interrupt_clear
	(
	int irq
	)
{
	
	return 0;
}

/* ================ */
/* termios */

void rtems_termios_enqueue_raw_characters(void *a, char *c, int l)  
{
}

void rtems_termios_rxirq_occured(void *a, char *c, int l)  
{
}

void rtems_termios_dequeue_characters(void *a, int l)
{
}

int rtems_io_register_name(char *name, int major, int minor) 
{
	int typ = 0;
	DBG("@rtems_io_register_name: register [%d:%d] as %s\n", major, minor, name);
	if (Device_drivers_cnt <= major) {
		return -1;
	}
	return mknod(name, Device_drivers[major].typ, major_minor(major, minor));
}

void rtems_termios_initialize(void) 
{

}
void rtems_termios_open(int major, int minor, void *arg, void *c)
{
}

int rtems_termios_read(void *args) 
{

}
int rtems_termios_write(void *args) 
{
	rtems_libio_rw_args_t *a = (rtems_libio_rw_args_t *) args;
	DBG("@rtems_termios_write: buffer 0x%x length 0x%x\n", a->buffer, a->count);
}

int rtems_termios_close(void *args) 
{

}

int rtems_termios_ioctl(void *args) 
{

}
