#ifndef RTEMSCOMPAT
#define RTEMSCOMPAT

#define RTEMS_DRVMGR_STARTUP	

#define rtems_drvmgr_dev_info drvmgr_dev
#define rtems_drvmgr_bus_info drvmgr_bus
#define rtems_drvmgr_drv_info drvmgr_drv
#define rtems_drvmgr_isr drvmgr_isr
#define rtems_drvmgr_drv_ops drvmgr_drv_ops
#define rtems_drvmgr_bus_params drvmgr_bus_params
#define rtems_drvmgr_dev_key_get drvmgr_dev_key_get
#define rtems_drvmgr_key_value drvmgr_key_value
#define rtems_drvmgr_drv_register drvmgr_drv_register
#define rtems_drvmgr_drv_res drvmgr_drv_res
#define rtems_drvmgr_root_drv_register drvmgr_root_drv_register
#define rtems_drvmgr_bus_ops drvmgr_bus_ops
#define rtems_drvmgr_mmap_entry drvmgr_mmap_entry
#define rtems_drvmgr_bus_res drvmgr_bus_res

#define rtems_drvmgr_interrupt_disable(a,b,c,d) drvmgr_interrupt_mask(a,b)
#define rtems_drvmgr_interrupt_enable(a,b,c,d) drvmgr_interrupt_unmask(a,b)
#define rtems_drvmgr_interrupt_clear(a,b,c,d) drvmgr_interrupt_clear(a,b)
#define rtems_drvmgr_interrupt_register(a,b,c,d) drvmgr_interrupt_register(a,b,0,c,d) 
#define rtems_drvmgr_get_dev_prefix drvmgr_get_dev_prefix
#define rtems_drvmgr_alloc_dev drvmgr_alloc_dev
#define rtems_drvmgr_bus_register drvmgr_bus_register
#define rtems_drvmgr_dev_register drvmgr_dev_register
#define rtems_drvmgr_alloc_bus drvmgr_alloc_bus
#define rtems_drvmgr_bus_res_add drvmgr_bus_res_add 
#define rtems_drvmgr_on_rootbus drvmgr_on_rootbus
#define rtems_drvmgr_interrupt_unregister drvmgr_interrupt_unregister
#define rtems_drvmgr_freq_get drvmgr_freq_get

#define rtems_interrupt_disable(a) (a=leonbare_disable_traps())
#define rtems_interrupt_enable(a) leonbare_enable_traps(a)
#define printk printf

#define rtems_status_code int
#define rtems_device_major_number int
#define rtems_device_minor_number int
#define rtems_device_driver rtems_status_code

#endif
