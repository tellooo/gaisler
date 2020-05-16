/*
 * RTEMS PCI demonstration application modified for
 * NGMP research project - demonstrate PCI where two NGMP boards are connected
 * over PCI.
 *
 *
 * The packets are marked with a unique sequence number and contain 16-bit
 * word incremented data, this is to be able to verify packet RX/TX ordering
 * and data correctness. The packet sequence is verified for every received
 * packet, and optionally the data may also be verified for every reception.
 *
 * The test runs for LENGTH_SEC seconds, when the time has expired the test
 * stops sending on the "packet initiator" instead it collects all transmitted
 * packet by waiting for WAIT_SEC for all packets to be received at the
 * initiator. If the test is successful all packets the initiator has sent has
 * also been received, finaly the packet sequence and data content is verified
 * at the end.
 *
 * The option to only verify data at the end makes the test zero-copy, the
 * SpaceWire packet data is not read by the CPU. This makes it possible to
 * achieve high bitrates.
 *
 *
 * Configuration options
 * ---------------------
 * When the application is run from GRMON, GRMON can be used to change the
 * operation. For example the GRPCI2 DMA channel and data buffers DMA location
 * can be altered, and the dma_cfg may be changed without rebuilding the
 * application:
 *
 * grmon> load test
 * grmon> wmem buf_chanbd_start 0x01100000
 * grmon> wmem buf_databd_start 0x01800000
 * grmon> wmem buf_data_start 0x02000000
 * grmon> wmem dma_cfg 8
 * grmon> stack 0x00ffffff
 * grmon> l2cache disable
 * grmon> run
 *
 */

/* Test setup */
/* Define PCI_POLLING for polling, otherwise and interrupt is used. */
//#define PCI_POLLING
/* Number of seconds to run Test */
#define TEST_LENGTH_SEC (10)
/* Size of each DMA transfer (MAX is 256*1024) */
/* Toal transfer size */
//#define TRANSFER_SIZE (128*256*1024)
#define TRANSFER_SIZE (256*1024)
#define BLOCK_SIZE (256*1024)
#define DMA_DATA_DESCRIPTORS 128

/*** END OF CONFIGURATION OPTIONS - start of test application ***/

#include <rtems.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */
#include <grlib/grpci2.h>
#include <grlib/grpci2dma.h>

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_MAXIMUM_SEMAPHORES        20
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_PERIODS             1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES    RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MICROSECONDS_PER_TICK     RTEMS_MILLISECONDS_TO_MICROSECONDS(2)

/* Configure PCI Library to auto configuration. This can be substituted with
 * a static configuration by setting PCI_LIB_STATIC, see pci/. Static
 * configuration can be generated automatically by print routines in PCI
 * library.
 */
#define RTEMS_PCI_CONFIG_LIB
/*#define CONFIGURE_PCI_LIB PCI_LIB_STATIC*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO
#include <pci.h>
#include <pci/irq.h>

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
/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_CPCI_GR740        /* GR-CPCI-GR740 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRIOMMU  /* GRIOMMU driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_L2CACHE  /* L2CACHE driver  */

/*******************************************/

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif


/******** ADD A CUSTOM PCI DRIVER **********/
/* Uncomment the line below to add the custom driver, don't forget to 
 * set PCIID_VENDOR_CUSTOM and PCIID_DEVICE_CUSTOM
 */
//#define CONFIGURE_DRIVER_CUSTOM1

//#define DRIVER_CUSTOM1_REG {gr_cpci_gr740_register_drv}
/*******************************************/


//#define DEBUG 1

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...) 
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "config.c"

#ifdef HOST_NGMP
#include "mmu_setup.h"
#endif

void printf_error(int ret, const char *msg){
    switch (ret) {
        case GRPCI2DMA_ERR_WRONGPTR:
            printf("%s FAILED with WRONPTR.\n", msg);
            break;
        case GRPCI2DMA_ERR_NOINIT:
            printf("%s FAILED with NOINIT.\n", msg);
            break;
        case GRPCI2DMA_ERR_TOOMANY:
            printf("%s FAILED with TOOMANY.\n", msg);
            break;
        case GRPCI2DMA_ERR_ERROR:
            printf("%s FAILED with ERROR.\n", msg);
            break;
        case GRPCI2DMA_ERR_STOPDMA:
            printf("%s FAILED with STOPDMA.\n", msg);
            break;
        case GRPCI2DMA_ERR_NOTFOUND:
            printf("%s FAILED with NOTFOUND.\n", msg);
            break;
        default:
            printf("%s FAILED with return value: %d.\n", msg, ret);
            break;
    }
}

int pci_length_secs=TEST_LENGTH_SEC;

#ifdef HOST_NGMP
/* Override from GRMON by "wmem buf_chanbd_start START_ADDRESS". 0 means
 * dynamically allocated buffers */
void *buf_chanbd_start = (void *)0x01100000; /* memory not used by RTEMS */
void *buf_databd_start = (void *)0x01800000;
void *buf_data_start = (void *)0x02000000;
#else
void *buf_chanbd_start = NULL;
void *buf_databd_start = NULL;
void *buf_data_start = NULL;
#endif

#ifdef HOST_NGMP
/* Address DMA regions setup has 8 different configurations:
 *
 * CFG1 - demo coherent system without L2-cache
 *   MMU disabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG2 - demo coherent system with L2-cache
 *   MMU disabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG3 - demo coherent MMU system without L2-cache
 *   MMU enabled
 *   L2 cache disabled
 *   Masters via CPU-bus
 *
 * CFG4 - demo coherent MMU system with L2-cache
 *   MMU enabled
 *   L2 cache enabled
 *   Masters via CPU-bus
 *
 * CFG5 - demo coherent system with L2-cache, DMA not cached
 *   MMU disabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG6 - demo coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters via CPU-bus
 *
 * CFG7 - demo non-coherent MMU system without L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache disabled
 *   Masters direct into MEM-bus
 *
 * CFG8 - demo non-coherent MMU system with L2-cache, DMA not cached
 *   MMU enabled
 *   L2 cache enabled, MTRR regions mark DMA uncached
 *   Masters direct into MEM-bus
 *
 *
 * NOTE that bootloader/GRMON must configure stack 0x00ffffff to avoid conflict
 *      with DMA region 0x01000000 - 0x07ffffff. See stack GRMON comand
 *
 * NOTE Normal RTEMS behaviour is CFG2, or CFG8 when I/O trafic redirected
 */
int dma_cfg = 2; /* Config (override from GRMON with "wmem dma_cfg 8") */
#endif


rtems_task test_app(rtems_task_argument ignored);

rtems_id tids[4];
rtems_id period;

struct pci_test_context_t {
    /* Data to send */
    int * data_origptr;
    int * data;
    int data_size;

    /* Base addresses of local and remote PCI addresses*/
    uint32_t pci_local_address;
    uint32_t pci_local_size;
    uint32_t pci_remote_address;
    uint32_t pci_remote_size;

    /* PCI DMA */
    int dma_init;
    int dma_chan_no;
    /* Pointers to descriptor buffers */
    void * dma_chanbd;
    void * dma_databd;
    int dma_databd_allocated;
    /* Maximum data descriptors */
    int dma_databd_max;
    int dma_databd_size;
    volatile int dma_isr_done;
    grpci2dma_isr_t dma_isr;

    /* Transfer information */
    int transfer_databd; /* Number of data descriptors used */
    int transfer_size; /* Size of each subtransfer */
    int transfer_max; /* Number of subtransfers needed */
    int transfer_idx; /* Index of current subtransfer */
    int transfer_completed; /* Number of transfers completed */

    /* Test variables */
    int shutdown;
};
typedef struct pci_test_context_t pci_test_context_t;
static pci_test_context_t test_context = {0};

void dma_isr(void *data, int cid, unsigned int status){
    pci_test_context_t * context = data;
    context->dma_isr_done=1;
    return;
}

int pci_test_init( 
        pci_test_context_t * context,
        int data_size,
        void * data_ptr,
        void * chanbd_ptr,
        void * databd_ptr,
        int databd_max,
        int databd_size,
        grpci2dma_isr_t dma_isr
        )
{
    int ret;

    /* Init data part */
    if (context->data == NULL){
        context->data_size = data_size;

        /* Allocate data array if needed */
        if (data_ptr == NULL){
            /* Align array to 256-bit boundary */
            context->data_origptr = malloc(context->data_size + 0x20);
            context->data = (int *) (((int) context->data_origptr + 0x20) & ~(0x1f));
        } else {
            context->data = data_ptr;
        }
        /* Check allocation */
        if (!context->data){
            printf("Wrong malloc of data array\n");
            return -1;
        }
        /* Check alignment */
        if ( ((unsigned int ) context->data) & (0x20-1)) {
            printf("Wrong allign of data array!\n");
            return -2;
        }

        /* Initialize array */
        int i;
        for (i=0; i<context->data_size/sizeof(int); i++){
            context->data[i] = i;
        }
    }

    /* Get local and remote base addresses. TODO: Make this automatic. */
    #ifdef TARGET_NGMP
        #ifdef HOST_NGMP
    context->pci_local_address = 0x88000000;
    context->pci_local_size = 0x7ffffff;
    context->pci_remote_address = 0x00000000;
    context->pci_remote_size = 0x7ffffff;
        #else
    context->pci_local_address = 0xc0000000;
    context->pci_local_size = 0x7ffffff;
    context->pci_remote_address = 0x00000000;
    context->pci_remote_size = 0x7ffffff;
        #endif
    #else
        #ifdef HOST_NGMP
    context->pci_local_address = 0x80000000;
    context->pci_local_size = 0x7ffffff;
    context->pci_remote_address = 0x40000000;
    context->pci_remote_size = 0x7ffffff;
        #else
    context->pci_local_address = 0xc0000000;
    context->pci_local_size = 0x7ffffff;
    context->pci_remote_address = 0x40000000;
    context->pci_remote_size = 0x7ffffff;
        #endif
    #endif
    if (context->pci_local_size < context->data_size){
        printf("Transfer is bigger than the remote BAR size.\n");
        return -3;
    }
    printf("Remote Memory (0x%08x) mapped to local 0x%08x\n", 
            (unsigned int) context->pci_remote_address,
            (unsigned int) context->pci_local_address);

    /* Init PCI DMA */
    if (context->dma_init == 0){
        context->dma_chanbd = chanbd_ptr;
        /* Open & allocate DMA channel */
        context->dma_chan_no = grpci2dma_open(context->dma_chanbd);
        if (context->dma_chan_no < 0) {
            printf_error(context->dma_chan_no, "Opening DMA channel ");
            return -4;
        }

        /* Register ISR for channel */
        context->dma_isr = dma_isr;
        grpci2dma_isr_register(context->dma_chan_no, context->dma_isr, context);
        context->dma_isr_done = 0;

        /* Allocate data descriptor buffer */
        context->dma_databd = databd_ptr;
        context->dma_databd_max = databd_max;
        context->dma_databd_size = databd_size;
        if (context->dma_databd == NULL) {
            context->dma_databd = grpci2dma_data_new(context->dma_databd_max);
            context->dma_databd_allocated = 1;
        }
        if (context->dma_databd == NULL) {
            printf("Data descriptor buffer allocationg failed\n");
            return -5;
        }

        /* Start DMA channel */
        ret = grpci2dma_start(context->dma_chan_no, 0);
        if (ret != GRPCI2DMA_ERR_OK){
            printf_error(ret, "Starting DMA ");
            return -6;
        }
        context->dma_init = 1;
    }

    /* Get how many transfers will be required to send all data */
    int maxtsize = (context->dma_databd_max * context->dma_databd_size);
    if (context->data_size > maxtsize){
        /* There will be more than 1 transfer */
        context->transfer_size = maxtsize;
        context->transfer_max = (context->data_size/maxtsize) + (context->data_size%(maxtsize)? 1:0);
        context->transfer_idx = 0;
        context->transfer_completed = 0;
    }else {
        context->transfer_size = context->data_size;
        context->transfer_max = 1;
        context->transfer_idx = 0;
        context->transfer_completed = 0;
    }
    context->shutdown = 0;

    return 0;
}

int pci_test_end( pci_test_context_t * context )
{
    /* Close channel */
    int ret = grpci2dma_close(context->dma_chan_no);
    if (ret != GRPCI2DMA_ERR_OK) {
        printf_error(ret, "Closing DMA channel ");
        return -1;
    }
    /* Deallocate data descriptors if needed */
    if (context->dma_databd_allocated != 0){
        grpci2dma_data_delete(context->dma_databd);
        context->dma_databd_allocated = 0;
        context->dma_databd = NULL;
    }
    context->dma_init = 0;
    /* Deallocate data */
    if (context->data_origptr != NULL){
        free(context->data_origptr);
        context->data_origptr = NULL;
        context->data = NULL;
    }
    context->shutdown = 0;

    return 0;
}

int pci_test_step( pci_test_context_t * context )
{
    int ret;
    int i;

    /* Compute addresses */
    uint32_t pci_adr = (uint32_t) context->pci_local_address + 
        (context->transfer_idx * context->transfer_size);
    uint32_t ahb_adr = (uint32_t) context->data + 
        (context->transfer_idx * context->transfer_size);
    uint32_t * dataptr = (uint32_t *) ahb_adr;

    /*** FIRST SEND LOCAL DATA ***/
    /* Prepare DMA descriptors */
    context->transfer_databd = grpci2dma_prepare(pci_adr, ahb_adr, 
            GRPCI2DMA_AHBTOPCI, GRPCI2DMA_BIGENDIAN, context->transfer_size, 
            context->dma_databd, 0, context->dma_databd_max, context->dma_databd_size);
    if (context->transfer_databd <= 0 ){
        printf_error(context->transfer_databd, "Preparing DMA transfer ");
        return -1;
    }
    /* Enable interrupt on last data */
    grpci2dma_interrupt_enable(context->dma_databd, context->transfer_databd-1, context->transfer_databd, GRPCI2DMA_OPTIONS_ONE);

    /* Push descriptors to channel*/
    ret = grpci2dma_push(context->dma_chan_no, context->dma_databd, 0, context->transfer_databd);
    if (ret != GRPCI2DMA_ERR_OK){
        printf_error(ret, "Pushing data into channel ");
        return -2;
    }

    /* Wait for transfer to finish */
    #ifdef PCI_POLLING
    while ( (context->shutdown == 0)  && (grpci2dma_status(context->dma_databd, 0, context->transfer_databd) != GRPCI2DMA_BD_STATUS_DISABLED)){
        #ifndef HOST_NGMP
        rtems_task_wake_after(7);
        #endif
    }
    #else
    while ( (context->shutdown == 0) && (context->dma_isr_done == 0) ) {}
    /* Reinitialize ISR variable */
    context->dma_isr_done = 0;
    #endif

    /* Check all packets were finished */
    ret = grpci2dma_status(context->dma_databd, 0, context->transfer_databd);
    if (ret != GRPCI2DMA_BD_STATUS_DISABLED){
        if (ret == GRPCI2DMA_BD_STATUS_ENABLED){
            printf("Not all transfer finished!\n");
        }else if (ret == GRPCI2DMA_BD_STATUS_ERR){
            printf("Error on transfer!\n");
        }
        return -3;
    }

    /*** SECOND CLEAR LOCAL DATA ***/
    for (i=0; i<context->transfer_size/sizeof(int); i++) {
        dataptr[i] = 0;
    }

    /*** THIRD READ REMOTE DATA ***/
    /* Prepare DMA descriptors */
    context->transfer_databd = grpci2dma_prepare(pci_adr, ahb_adr, 
            GRPCI2DMA_PCITOAHB, GRPCI2DMA_BIGENDIAN, context->transfer_size, 
            context->dma_databd, 0, context->dma_databd_max, context->dma_databd_size);
    if (context->transfer_databd <= 0 ){
        printf_error(context->transfer_databd, "Preparing DMA transfer ");
        return -1;
    }
    /* Enable interrupt on last data */
    grpci2dma_interrupt_enable(context->dma_databd, context->transfer_databd-1, context->transfer_databd, GRPCI2DMA_OPTIONS_ONE);

    /* Push descriptors to channel*/
    ret = grpci2dma_push(context->dma_chan_no, context->dma_databd, 0, context->transfer_databd);
    if (ret != GRPCI2DMA_ERR_OK){
        printf_error(ret, "Pushing data into channel ");
        return -2;
    }

    /* Wait for transfer to finish */
    #ifdef PCI_POLLING
    while ( (context->shutdown == 0)  && (grpci2dma_status(context->dma_databd, 0, context->transfer_databd) != GRPCI2DMA_BD_STATUS_DISABLED)){
        #ifndef HOST_NGMP
        rtems_task_wake_after(7);
        #endif
    }
    #else
    while ( (context->shutdown == 0) && (context->dma_isr_done == 0) ) {}
    /* Reinitialize ISR variable */
    context->dma_isr_done = 0;
    #endif

    /* Check all packets were finished */
    ret = grpci2dma_status(context->dma_databd, 0, context->transfer_databd);
    if (ret != GRPCI2DMA_BD_STATUS_DISABLED){
        if (ret == GRPCI2DMA_BD_STATUS_ENABLED){
            printf("Not all transfer finished!\n");
        }else if (ret == GRPCI2DMA_BD_STATUS_ERR){
            printf("Error on transfer!\n");
        }
        return -3;
    }

    /*** NOW CHECK THAT THE DATA RECEIVED IS STILL VALID ***/
    ret = 0;
    for (i=0; i<context->transfer_size/sizeof(int); i++) {
        uint32_t got = dataptr[i];
        uint32_t exp = i + (context->transfer_idx * context->transfer_size)/sizeof(int);
        if (got != exp) {
            printf("Data (0x%08x) check failed! i:%d, Got(%d)=0x%08x, Exp(%d)=0x%08x\n", 
                    (unsigned int) dataptr, 
                    i, 
                    i, 
                    (unsigned int) got, 
                    (int) exp, 
                    (unsigned int) exp);
            printf("Tranfer details: IDX:%d, TXSIZE:%d, MAX:%d, COMPL:%d, DATABD:%d\n",
                context->transfer_idx,
                context->transfer_size,
                context->transfer_max,
                context->transfer_completed,
                context->transfer_databd
                );
            ret = 1;
            break;
        }
    }
    if (ret != 0) {
        return ret;
    }

    /*** STEP DONE ***/
    context->transfer_idx++;
    if (context->transfer_idx == context->transfer_max){
        context->transfer_completed++;
        context->transfer_idx = 0;
    }

    return 0;
}

rtems_task Init(
  rtems_task_argument ignored
)
{
    /* Initialize Driver manager and Networking, in config.c */
    system_init();

    /* Print device topology */
    drvmgr_print_topo();

    if (rtems_task_ident(RTEMS_SELF,0, &tids[0]) != RTEMS_SUCCESSFUL) {
        printf("Init task not identified!\n");
    }

    /* Run SpaceWire Test application */
    rtems_task_create(
            rtems_build_name( 'T', 'A', '0', '1' ),
            10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
            RTEMS_FLOATING_POINT, &tids[1]);
    rtems_task_start(tids[1], test_app, 0);
    rtems_task_suspend( RTEMS_SELF );
    /* TEST_APP task finished */
    exit(0);
}

#undef DEBUG_TEST

#ifdef DEBUG_TEST
#define PRINT(str...) printf(str); fflush(NULL)
#define PRINT2(str...) printf(str); fflush(NULL)
#define PRINT3(str...) printf(str); fflush(NULL)
#else
#define PRINT(str...)
#define PRINT2(str...)
#define PRINT3(str...)
#endif

rtems_task test_app(rtems_task_argument ignored)
{
    int tot, ret, stop;
    struct timespec t0, t1;
    double time;
    pci_test_context_t * context = &test_context;

    printf("\nTest built for PCI.\n"
            "System clock: %" PRIu32 " us / tick\n"
            "Test will run for %d s.\n",
            rtems_configuration_get_microseconds_per_tick(),
            pci_length_secs);
    printf("\n\n");


    #ifdef HOST_NGMP
    printf("DMA configuration number: %d\n", dma_cfg);
    switch (dma_cfg) {
        case 1:
            address_region_setup(MMU_DISABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        default:
        case 2:
            address_region_setup(MMU_DISABLE, L2C_ENABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        case 3:
            address_region_setup(MMU_ENABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        case 4:
            address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_DISABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        case 5:
            address_region_setup(MMU_DISABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        case 6:
            address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_CPU|IOMMU_PREFETCH_ENABLE);
            break;
        case 7:
            address_region_setup(MMU_ENABLE, L2C_DISABLE | L2C_MTRR_DISABLE, IOMMU_BUS_MEM|IOMMU_PREFETCH_DISABLE);
            break;
        case 8:
            address_region_setup(MMU_ENABLE, L2C_ENABLE | L2C_MTRR_ENABLE, IOMMU_BUS_MEM|IOMMU_PREFETCH_DISABLE);
            break;
    }
    #endif

    /* Packet processing loop */
    stop = 0;

    /* Init test */
    ret = pci_test_init(context, TRANSFER_SIZE, 
            buf_data_start, buf_chanbd_start, buf_databd_start, 
            DMA_DATA_DESCRIPTORS, BLOCK_SIZE, dma_isr);
    if (ret != 0){
        printf("Failure when initializing test context: %d\n", ret);
        stop = 1;
    }

    /* Start counting time */
    rtems_clock_get_uptime(&t0);

    while ( stop == 0 ) {
        /* Check if total seconds has gone now and then do stop sequence */
        rtems_clock_get_uptime(&t1);
        if ( t1.tv_sec > (t0.tv_sec + pci_length_secs) ) {
            stop = 1;
            break;
        }
        /* Execute test step */
        ret = pci_test_step(context);
        if (ret != 0){
            printf("Failure on test step: %d\n", ret);
            stop = 1;
        }
    }

    /* Stop time */
    rtems_clock_get_uptime(&t1);

    /* End test */
    ret = pci_test_end(context);
    if (ret != 0){
        printf("Failure on test context end: %d\n", ret);
    }

    time = (float)t1.tv_sec + (float)t1.tv_nsec/1000000000;
    time = time - (float)t0.tv_sec - (float)t0.tv_nsec/1000000000;
    printf(" Time:          %f seconds\n", time);

    /* Get Stats, this is to determine how many data have
     * been transfered.
     */
    tot = (context->transfer_completed * context->data_size) +
        (context->transfer_idx * context->transfer_size);
    /* Since for every transfer we are sending and receiving data, 
     * we need to multiply by 2 */
    tot = 2*tot;

    printf("\n\nShutting down: %d\n\n", context->shutdown);
    printf(" t0.tv_sec:     %u\n", (unsigned int)t0.tv_sec);
    printf(" t0.tv_nsec:    %ld\n", t0.tv_nsec);
    printf(" t1.tv_sec:     %u\n", (unsigned int)t1.tv_sec);
    printf(" t1.tv_nsec:    %ld\n", t1.tv_nsec);
    time = (float)t1.tv_sec + (float)t1.tv_nsec/1000000000;
    time = time - (float)t0.tv_sec - (float)t0.tv_nsec/1000000000;
    printf(" Time:          %f seconds\n", time);
    printf(" Bytes sent:    %d bytes\n", tot);
    printf(" Data desc:     %d desc\n", context->dma_databd_max);
    printf(" Block size:    %d bytes\n", context->dma_databd_size);
    printf(" Transfers:     %d transfers\n", context->transfer_max);
    printf(" Throughput:    %f bytes/sec\n", ((float)tot)/time);
    printf(" Throughput:    %f Kbytes/sec\n", ((((float)tot))/1000)/time);
    printf(" Throughput:    %f Kbits/sec\n", 8 * ((((float)tot))/1000)/time);

    printf("\n\n");

    rtems_id id = tids[0];
    rtems_task_resume(id);
    rtems_task_delete(RTEMS_SELF);
}

#undef DEBUG
