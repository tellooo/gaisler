/*
 * A RTEMS sample application accessing the SpaceWire router PCI design. The
 * PCIF, GRPCI, GRPCI2 or AT697 PCI Host driver is used to interface to
 * the PCI bus.
 *
 * SpW ports 1-2, 3-4, 5-6 and 7-8 should be connected with SpW cables. 
 * The test opens file handles to the configuration port and the two 
 * AMBA ports. First two packets are transferred from the AMBA ports through 
 * all of the active SpW links and finally received back to the AMBA ports. 
 * The second test is similar but uses packet distribution. 
 */

#include <rtems.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_MAXIMUM_TASKS             4

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_EXTRA_TASK_STACKS         (10 * RTEMS_MINIMUM_STACK_SIZE)

#define CONFIGURE_MAXIMUM_DRIVERS 16
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define RTEMS_PCI_CONFIG_LIB
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

#include <rtems/confdefs.h>

/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) /* if --drvmgr was given to configure */
 /* Add Timer and UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif
#endif
#ifdef LEON3
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF    /* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#endif
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_ADCDAC    /* GR-RASTA-ADCDAC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_SPW_ROUTER  /* GR-RASTA-SPW-ROUTER PCI Target Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW /* GRSPW driver for ROUTER AMBA ports */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* GRSPWROUTER configuration port driver */
#define CONFIGURE_DRIVER_PCI_GR_LEON4_N2X       /* GR-CPCI-LEON4-N2X PCI peripheral driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_AHBSTAT /* AHBSTAT driver */

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <grlib/grspw_router.h>
#include <grlib/grspw.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

#define GRSPWRTR_DEVICE_NAME "/dev/spwrouter0/router0"
#define AMBA0_DEVICE_NAME "/dev/spwrouter0/grspw0"
#define AMBA1_DEVICE_NAME "/dev/spwrouter0/grspw1"

#define IOCTL(fd,num,arg) \
	{ \
  	if ( ioctl(fd,num,arg) != RTEMS_SUCCESSFUL ) { \
			printf("ioctl " #num " failed: errno: %d\n",errno); \
      return -1; \
		} \
  }

int router_init(void);

rtems_task Init(
  rtems_task_argument ignored
)
{

	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */	
	drvmgr_print_topo();

	printf("\n\n ####### PCI CONFIGURATION #######\n\n");

	/* Print PCI bus resources */
	pci_print();

        /* Print devices and drivers */
        drvmgr_print_devs(PRINT_DEVS_ALL); 

#ifdef LEON3
	printf("\n\n ####### AMBA PnP CONFIGURATION #######\n\n");

	/* Print AMBA Bus */
	ambapp_print(&ambapp_plb, 1);
#endif

        /* Init router test application */
        router_init();

	exit( 0 );
}
int router_init(void) {
                int i;
                int j;
                int ret;
                int len;
                int txlen;
                int txlen2;
                int rtrfd;
                int amba0fd;
                int amba1fd;
                int nports;
                char *tx_pkt0;
                char *tx_pkt1;
                char *rx_pkt0;
                char *rx_pkt1;
                
                struct router_hw_info hw_info;
                struct router_config rtr_cfg;
                struct router_port port;
                struct router_ps ps;
                struct router_routes routes;
                
                rtrfd = open(GRSPWRTR_DEVICE_NAME, O_RDWR);
                if ( rtrfd < 0 ){
                        printf("Failed to open " GRSPWRTR_DEVICE_NAME " driver (%d)\n", errno);
                        return -1;
                }

                printf("--- ROUTER HARDWARE INFO ---\n");
                
                IOCTL(rtrfd, GRSPWR_IOCTL_HWINFO, &hw_info);
                printf("SpW Ports: %d\n", hw_info.nports_spw);
                printf("AMBA Ports: %d\n", hw_info.nports_amba);
                printf("FIFO Ports: %d\n", hw_info.nports_fifo);
                
                nports = hw_info.nports_spw + hw_info.nports_amba + hw_info.nports_fifo;
                
                if (hw_info.timers_avail) {
                        printf("Timers available\n");
                } else {
                        printf("Timers not available\n");
                }

                if (hw_info.pnp_avail) {
                        printf("PnP available\n");
                } else {
                        printf("PnP not available\n");
                }
                
                printf("Major Version: %d\n", hw_info.ver_major);
                printf("Minor Version: %d\n", hw_info.ver_minor);
                printf("Patch Version: %d\n", hw_info.ver_patch);
                printf("Instance ID: %d\n", hw_info.iid);

                IOCTL(rtrfd, GRSPWR_IOCTL_CFG_GET, &rtr_cfg);

                printf("Configuration register: %x\n", rtr_cfg.config);
                printf("Instance ID: %x\n", rtr_cfg.iid);
                printf("Initialization Divisor: %d\n", rtr_cfg.idiv);
                printf("Timer Prescaler: %x\n", rtr_cfg.timer_prescaler);
                for (i = 0; i <= nports; i++) {
                        printf("Timer Reload[%d]: %x\n", i, rtr_cfg.timer_reload[i]);
                }

                
                printf("--- CHANGE TIMER VALUES ---\n");
                rtr_cfg.flags = ROUTER_FLG_TPRES | ROUTER_FLG_TRLD;
                rtr_cfg.timer_prescaler = 50;
                for (i = 0; i < 32; i++) {
                        rtr_cfg.timer_reload[i] = 10;
                }
                
                IOCTL(rtrfd, GRSPWR_IOCTL_CFG_SET, &rtr_cfg);

                IOCTL(rtrfd, GRSPWR_IOCTL_CFG_GET, &rtr_cfg);

                printf("Configuration register: %d\n", rtr_cfg.config);
                printf("Instance ID: %d\n", rtr_cfg.iid);
                printf("Initialization Divisor: %d\n", rtr_cfg.idiv);
                printf("Timer Prescaler: %x\n", rtr_cfg.timer_prescaler);
                for (i = 0; i <= nports; i++) {
                        printf("Timer Reload[%d]: %x\n", i, rtr_cfg.timer_reload[i]);
                }

                printf("--- PORT CTRL AND STATUS ---\n");
                for(i = 1; i <= nports; i++) {
                        port.flag = ROUTER_PORTFLG_GET_CTRL | ROUTER_PORTFLG_GET_STS;
                        port.port = i;
                        IOCTL(rtrfd, GRSPWR_IOCTL_PORT, &port);
                        printf("Port[%d] CTRL: %x\n", i, port.ctrl);
                        printf("Port[%d] STATUS: %x\n", i, port.sts);
                        
                        if ((i == 1) || (i == 3) || (i == 5) || (i == 7)) {
                                printf("Enable and Start Link: %d\n", i);
                                port.flag = ROUTER_PORTFLG_SET_CTRL;
                                port.ctrl = (port.ctrl & 0xFFFFFFF8) | 0x6;
                                IOCTL(rtrfd, GRSPWR_IOCTL_PORT, &port);
                                port.flag = ROUTER_PORTFLG_GET_CTRL | ROUTER_PORTFLG_GET_STS;
                                IOCTL(rtrfd, GRSPWR_IOCTL_PORT, &port);
                                printf("Port[%d] CTRL: %x\n", i, port.ctrl);
                                printf("Port[%d] STATUS: %x\n", i, port.sts);
                        }
                        printf("\n\n");
                }
                
                printf("--- OPEN AMBA PORT 0 ---\n");
                amba0fd = open(AMBA0_DEVICE_NAME, O_RDWR);
                if ( amba0fd < 0 ){
                        printf("Failed to open " AMBA0_DEVICE_NAME " driver (%d)\n", errno);
                        return -1;
                }
                IOCTL(amba0fd, SPACEWIRE_IOCTRL_SET_NODEADDR, 1);
                IOCTL(amba0fd, SPACEWIRE_IOCTRL_START, 0);
                
                
                printf("--- OPEN AMBA PORT 1 ---\n");
                amba1fd = open(AMBA1_DEVICE_NAME, O_RDWR);
                if ( amba1fd < 0 ){
                        printf("Failed to open " AMBA1_DEVICE_NAME " driver (%d)\n", errno);
                        return -1;
                }
                IOCTL(amba1fd, SPACEWIRE_IOCTRL_SET_NODEADDR, 2);
                IOCTL(amba1fd, SPACEWIRE_IOCTRL_START, 0);

                /* IOCTL(amba0fd, SPACEWIRE_IOCTRL_SET_RXBLOCK, 1); */
/*                 IOCTL(amba1fd, SPACEWIRE_IOCTRL_SET_RXBLOCK, 1); */
                
                printf("--- SEND AND RECEIVE PACKETS ---\n");
                tx_pkt0 = malloc(1024);
                tx_pkt1 = malloc(1024);
                rx_pkt0 = malloc(1024);
                rx_pkt1 = malloc(1024);
                tx_pkt0[0] = 1;
		tx_pkt0[1] = 3;
                tx_pkt0[2] = 5;
                tx_pkt0[3] = 7;
                tx_pkt0[4] = 17;
                tx_pkt0[5] = 1;
                tx_pkt1[0] = 1;
		tx_pkt1[1] = 3;
                tx_pkt1[2] = 5;
                tx_pkt1[3] = 7;
                tx_pkt1[4] = 18;
                tx_pkt1[5] = 2;
                
                for (i = 0; i < 20; i++) {
                        printf("Iteration: %d\n", i);
                        txlen = rand() % 1024;
                        txlen2 = rand() % 1024;
                        printf("Tx Len[0]: %d\n", txlen);
                        printf("Tx Len[1]: %d\n", txlen2);
                        if (txlen < 10) {
                                txlen = 10;
                        }
                        if (txlen2 < 10) {
                                txlen2 = 10;
                        }
                        for (j = 6; j < txlen; j++) {
                                tx_pkt0[j] = rand() % 256;
                        }
                        for (j = 6; j < txlen2; j++) {
                                tx_pkt1[j] = rand() % 256;
                        }
                        if ( (ret=write(amba0fd, tx_pkt0, txlen)) <= 0 ){
                                printf("Write failed, errno: %d, ret: %d\n",errno,ret);
                        }
                        if ( (ret=write(amba1fd, tx_pkt1, txlen2)) <= 0 ){
                                printf("Write failed, errno: %d, ret: %d\n",errno,ret);
                        }
                        len = 0;
                        while(1) {
                                ret=read(amba0fd, rx_pkt0, txlen);
                                if ( ret <= 0 ){
                                        printf("Read Failed, errno: %d, ret: %d\n",errno,ret);
                                        sleep(1);
                                        continue;
                                } else {
                                        for (j = len; j < len+ret; j++) {
                                                if (tx_pkt0[j+5] != rx_pkt0[j]) {
                                                        printf("ERROR: Incorrect data received. Exp: %x, Got: %x\n", tx_pkt0[j+5], rx_pkt0[j]);
                                                }
                                        }
                                        len = len + ret;
                                        if (len >= txlen-5) {
                                                printf("Packet of length: %d received on link 1\n", len);
                                                break;
                                        }
                                        
                                }
                        }
                        len = 0;
                        while(1) {
                                ret=read(amba1fd, rx_pkt1, txlen2);
                                if ( ret <= 0 ){
                                        printf("Read Failed, errno: %d, ret: %d\n",errno,ret);
                                        sleep(1);
                                        continue;
                                } else {
                                        for (j = len; j < len+ret; j++) {
                                                if (tx_pkt1[j+5] != rx_pkt1[j]) {
                                                        printf("ERROR: Incorrect data received. Exp: %x, Got: %x\n", tx_pkt1[j+5], rx_pkt1[j]);
                                                }
                                        }
                                        len = len + ret;
                                        if (len >= txlen2-5) {
                                                printf("Packet of length: %d received on link 2\n", len);
                                                break;
                                        }
                                        
                                }
                        }
                        
                }

                printf("--- ENABLE PACKET DISTRIBUTION FOR PHYSICAL ADDRESS 1 ---\n");
                for(i = 0; i < 31; i++) {
                        ps.ps[i] = 0;
                        ps.ps_logical[i] = 0;
                }
                for(i = 31; i < 224; i++) {
                        ps.ps_logical[i] = 0;
                }

                ps.ps[0] = 0x1F;
                
                IOCTL(rtrfd, GRSPWR_IOCTL_PS_SET, &ps);
                
                for(i = 0; i < 224; i++) {
                        routes.route[i] = 0;
                }
                
                IOCTL(rtrfd, GRSPWR_IOCTL_ROUTES_SET, &routes);
                
                IOCTL(rtrfd, GRSPWR_IOCTL_PS_GET, &ps);

                for(i = 1; i < 256; i++) {
                        if (i < 32) {
                                printf("Physical address[%d]: %x\n", i, ps.ps[i-1]);
                        } else {
                                printf("Logical address[%d]: %x\n", i, ps.ps_logical[i-32]);
                        }
                }
                
                tx_pkt0[0] = 1;
                tx_pkt0[1] = 17;
                tx_pkt0[2] = 1;
                for (i = 3; i < 64; i++) {
                        tx_pkt0[i] = rand() % 256;
                }
                
                if ( (ret=write(amba0fd, tx_pkt0, 64)) <= 0 ){
                        printf("Write failed, errno: %d, ret: %d\n",errno,ret);
                }
                for (i = 0; i < 4; i++) {
                        printf("Receiving packet: %d\n", i);
                        len = 0;
                        while(1) {
                                ret=read(amba0fd, rx_pkt0, 62);
                                if ( ret <= 0 ){
                                        printf("Read Failed, errno: %d, ret: %d\n",errno,ret);
                                        sleep(1);
                                        continue;
                                } else {
                                        for (j = len; j < len+ret; j++) {
                                                if (tx_pkt0[j+2] != rx_pkt0[j]) {
                                                        printf("ERROR: Incorrect data received. Exp: %x, Got: %x\n", tx_pkt0[j+2], rx_pkt0[j]);
                                                }
                                        }
                                        len = len + ret;
                                        if (len >= 62) {
                                                printf("Packet of length: %d received on link 1\n", len);
                                                break;
                                        }
                                        
                                }
                        }
                }

                ps.ps[0] = 0;
                IOCTL(rtrfd, GRSPWR_IOCTL_PS_SET, &ps);

                while(1);
}
        
                        
