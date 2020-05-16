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

#include <rtems.h>
#include <stdlib.h>
#include <string.h>

#include <bsp.h> /* for device driver prototypes */
#include <grlib/grpci2.h>
#include <grlib/grpci2dma.h>

/*#define DEBUG 1*/

#ifdef DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...) 
#endif

#include <drvmgr/drvmgr_confdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include "pci_test.h"

#ifdef HOST_NGMP
/* Override from GRMON by "wmem buf_chanbd_start START_ADDRESS". 0 means
 * dynamically allocated buffers */
void *pci_buf_chanbd_start = (void *)0x03100000; /* memory not used by RTEMS */
void *pci_buf_databd_start = (void *)0x03800000;
void *pci_buf_data_start = (void *)0x04000000;
#else
void *pci_buf_chanbd_start = NULL;
void *pci_buf_databd_start = NULL;
void *pci_buf_data_start = NULL;
#endif

static void printf_error(int ret, const char *msg){
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

/* This function counts the bits of a value. 
 * Used to compute the errors on the IOMMU data*/
static unsigned int countBits( unsigned int n){
    unsigned int count = 0;
    while(n)
    {
        count += n & 1;
        n >>= 1;
    }
    return count;
}


struct pci_test_context_t {
    /* Data to send */
    int * data_origptr;
    int * data;
    int * data_write;
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
    int transfer_direction; /* Direction of transfer 0: Write to remote, 1: Read from remote */
    int transfer_check; /* Check if the trasnfer was correct */
    int transfer_error_wordcount; /* Number of words affected */
    int transfer_error_bitcount; /* Number of bits affected */

    /* Test variables */
    int shutdown;
    int start_trigger;
    int stop_trigger;
    int task_stop_trigger;
    int finished;
    rtems_id task_id;
    rtems_id timeout_sem;

    /* Results */
    int bytes_sent;
    struct timespec tstart, tstop;
};
typedef struct pci_test_context_t pci_test_context_t;
static pci_test_context_t pci_test_context = {0};

static void dma_isr(void *data, int cid, unsigned int status){
    pci_test_context_t * context = data;
    context->dma_isr_done=1;
    rtems_semaphore_release(context->timeout_sem);
    rtems_task_resume(context->task_id);
    return;
}

static int pci_test_init( 
        pci_test_context_t * context,
        int data_size,
        void * data_ptr_read,
        void * data_ptr_write,
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

        /* Allocate data array for read if needed */
        if (data_ptr_read == NULL){
            /* Align array to 256-bit boundary */
            context->data_origptr = malloc(context->data_size + 0x20);
            context->data = (int *) (((int) context->data_origptr + 0x20) & ~(0x1f));
        } else {
            context->data = data_ptr_read;
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

        /* Assign data array for write */
        if (data_ptr_write == NULL){
            /* Use the same array as for read */
            context->data_write = data_ptr_read;
        } else {
            context->data_write = data_ptr_write;
        }
        /* Check allocation */
        if (!context->data_write){
            printf("Wrong malloc of data write array\n");
            return -1;
        }
        /* Check alignment */
        if ( ((unsigned int ) context->data_write) & (0x20-1)) {
            printf("Wrong allign of data write array!\n");
            return -2;
        }

        /* Initialize read array */
        int i;
        for (i=0; i<context->data_size/sizeof(int); i++){
            context->data[i] = i;
        }
    }
    printf("PCI TEST: AHB read @:0x%08x, AHB write @:0x%08x, data size: %d bytes.\n", 
            (unsigned int) context->data, 
            (unsigned int) context->data_write, 
            context->data_size);

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
    printf("PCI TEST: PCI local @:0x%08x, PCI remote @:0x%08x.\n",
            (unsigned int) context->pci_local_address,
            (unsigned int) context->pci_remote_address);

    /* Init PCI DMA */
    if (context->dma_init == 0){
        context->dma_chanbd = chanbd_ptr;
        /* Open & allocate DMA channel */
        context->dma_chan_no = grpci2dma_open(context->dma_chanbd);
        if (context->dma_chan_no < 0) {
            printf_error(context->dma_chan_no, "Opening DMA channel ");
            return -4;
        }
        DBG("PCI TEST: Openend DMA channel %d.\n", context->dma_chan_no);

        /* Register ISR for channel */
        context->dma_isr = dma_isr;
        grpci2dma_isr_register(context->dma_chan_no, context->dma_isr, context);
        context->dma_isr_done = 0;
        DBG("PCI TEST: Registered ISR.\n");

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
        DBG("PCI TEST: Started DMA channel.\n");
    }
    printf("PCI TEST: DMA chan: %d, DMA chanbd @: 0x%08x.\n",
            context->dma_chan_no,
            (unsigned int) context->dma_chanbd);
    printf("PCI TEST: DMA databd @: 0x%08x (max %d desc, %d bytes each).\n", 
            (unsigned int) context->dma_databd,
            context->dma_databd_max, context->dma_databd_size);


    /* Get how many transfers will be required to send all data */
    int maxtsize = (context->dma_databd_max * context->dma_databd_size);
    if (context->data_size > maxtsize){
        /* There will be more than 1 transfer */
        context->transfer_size = maxtsize;
        context->transfer_max = (context->data_size/maxtsize) + (context->data_size%(maxtsize)? 1:0);
    }else {
        context->transfer_size = context->data_size;
        context->transfer_max = 1;
    }
    context->transfer_idx = 0;
    context->transfer_completed = 0;
    context->transfer_direction = 0;
    context->transfer_check = 0;
    context->transfer_error_wordcount = 0;
    context->transfer_error_bitcount = 0;
    context->shutdown = 0;
    context->stop_trigger = 0;
    context->finished = 0;
    printf("PCI TEST: Transfers: %d, Transfer size: %d bytes\n", 
            context->transfer_max,
            context->transfer_size);

    /* Init results */
    context->bytes_sent = 0;
    memset(&context->tstart, 0, sizeof(struct timespec));
    memset(&context->tstop, 0, sizeof(struct timespec));

    return 0;
}

static int pci_test_end( pci_test_context_t * context )
{
    /* Check that the data array has the correct sequence */
    if (context->data_write != NULL){
        /* Initialize array */
        context->transfer_check = 1;
        int i, bits;
        for (i=0; i<context->data_size/sizeof(int); i++){
            if (context->data_write[i] != i) {
                context->transfer_check = 0;
                context->transfer_error_wordcount++;
                unsigned int diff = (unsigned int) (context->data_write[i] ^ i);
                bits = countBits(diff);
                context->transfer_error_bitcount += bits;
            }
        }
    }else{
        context->transfer_check = -1;
    }

    /* Close channel */
    int ret = grpci2dma_close(context->dma_chan_no);
    if (ret != GRPCI2DMA_ERR_OK) {
        printf_error(ret, "Closing DMA channel ");
        return -1;
    }
    DBG("PCI TEST: Closed DMA channel.\n");
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
    context->data_write = NULL;
    context->shutdown = 1;
    context->start_trigger = 0;

    return 0;
}

static int pci_test_step( pci_test_context_t * context )
{
    int ret;
    int dir;
    int sts;
    uint32_t adr;

    DBG("PCI TEST: Transfer %d of %d.\n", context->transfer_idx, context->transfer_max);

    /* Prepare transfer direction */
    if (context->transfer_direction == 0){
        /* Write to PCI (read from AHB)*/
        dir = GRPCI2DMA_AHBTOPCI;
        adr = (uint32_t) context->data;
    }else{
        /* Read from PCI (write to AHB)*/
        dir = GRPCI2DMA_PCITOAHB;
        adr = (uint32_t) context->data_write;
    }

    /* Compute addresses */
    uint32_t pci_adr = (uint32_t) context->pci_local_address + 
        (context->transfer_idx * context->transfer_size);
    uint32_t ahb_adr = (uint32_t) adr + 
        (context->transfer_idx * context->transfer_size);
    /* Compute remaining transfer size */
    uint32_t transfer_size;
    if (context->transfer_idx == context->transfer_max - 1){
        /* Last transfer */
        transfer_size = context->data_size - (context->transfer_idx*context->transfer_size);
    } else {
        transfer_size = context->transfer_size;
    }

    /* Prepare DMA descriptors */
    context->transfer_databd = grpci2dma_prepare(pci_adr, ahb_adr, 
            dir, GRPCI2DMA_BIGENDIAN, transfer_size, 
            context->dma_databd, 0, context->dma_databd_max, context->dma_databd_size);
    if (context->transfer_databd <= 0 ){
        printf_error(context->transfer_databd, "Preparing DMA transfer ");
        return -1;
    }
    DBG("PCI TEST: Prepared transfer (dir=%d): 0x%08x-0x%08x. Desc: %d of %d.\n", 
            context->transfer_direction,
            (unsigned int) pci_adr, (unsigned int) ahb_adr,
            context->transfer_databd, context->dma_databd_max);
    /* Enable interrupt on last data */
    grpci2dma_interrupt_enable(context->dma_databd, context->transfer_databd-1, context->transfer_databd, GRPCI2DMA_OPTIONS_ONE);
    DBG("PCI TEST: Interrupt enabled on desc: %d\n", context->transfer_databd -1);

    /* Push descriptors to channel*/
    ret = grpci2dma_push(context->dma_chan_no, context->dma_databd, 0, context->transfer_databd);
    if (ret != GRPCI2DMA_ERR_OK){
        printf_error(ret, "Pushing data into channel ");
        return -2;
    }
    DBG("PCI TEST: Descriptors pushed to DMA channel\n");

    /* Wait for transfer to finish */
    #ifdef PCI_POLLING
    while ( (context->shutdown == 0)  && (grpci2dma_status(context->dma_databd, 0, context->transfer_databd) != GRPCI2DMA_BD_STATUS_DISABLED)){
        rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }
    #else
    sts = rtems_semaphore_obtain(context->timeout_sem, RTEMS_WAIT, PCI_TIMEOUT);
    if (sts == RTEMS_TIMEOUT) {
        printf("PCI TEST: Timeout!\n");
        rtems_semaphore_flush(context->timeout_sem);
        return -3;
    } else if (sts != RTEMS_SUCCESSFUL) {
        return -4;
    }
    while ( (context->shutdown == 0) && (context->dma_isr_done == 0) ) {
        //rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
        rtems_task_suspend(RTEMS_SELF);
    }
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
        return -5;
    }
    DBG("PCI TEST: Descriptors finsihed.\n");

    /* Increment index */
    context->transfer_idx++;
    if (context->transfer_idx == context->transfer_max){
        /* Transfer completed */
        context->transfer_completed++;
        context->transfer_idx = 0;
        /* Change direction */
        if (context->transfer_direction == 0){
            context->transfer_direction = 1;
        } else {
            context->transfer_direction = 0;
        }
    }

    return 0;
}

static int pci_test_results( pci_test_context_t * context )
{
    /* Get Stats, this is to determine how many data have
     * been transfered.
     */
    int tot = context->bytes_sent;
    double time;
    time = (float)context->tstop.tv_sec + (float)context->tstop.tv_nsec/1000000000;
    time = time - (float)context->tstart.tv_sec - (float)context->tstart.tv_nsec/1000000000;

    printf("\n\n PCI TEST RESULTS:\n");
    printf(" t0.tv_sec:     %u\n", (unsigned int)context->tstart.tv_sec);
    printf(" t0.tv_nsec:    %ld\n", context->tstart.tv_nsec);
    printf(" t1.tv_sec:     %u\n", (unsigned int)context->tstop.tv_sec);
    printf(" t1.tv_nsec:    %ld\n", context->tstop.tv_nsec);
    printf(" Time:          %f seconds\n", time);
    printf(" Bytes sent:    %d bytes\n", tot);
    printf(" Data desc:     %d desc\n", context->dma_databd_max);
    printf(" Block size:    %d bytes\n", context->dma_databd_size);
    double transfers = (float) context->transfer_completed + 
        ( (float)context->transfer_idx/context->transfer_max);
    printf(" Transfers:     %f transfers\n", transfers);
    printf(" Throughput:    %f bytes/sec\n", ((float)tot)/time);
    printf(" Throughput:    %f bytes/sec\n", ((((float)tot)))/time);
    printf(" Throughput:    %f bits/sec\n", 8 * ((((float)tot)))/time);
    printf("#PCIRESULT=%f\n", 8 * ((((float)tot)))/time);

    if ( context->transfer_check != 1 ) {
        printf("###PACKET CONTENT/PKT-SEQUENCE CHECK FAILED: %d\n", context->transfer_check);
        printf(" Error W-count: %d words\n", context->transfer_error_wordcount);
        printf(" Error B-count: %d bits\n", context->transfer_error_bitcount);
    } else {
        printf("PACKET CONTENT/PKT-SEQUENCE CHECK SUCCESSFUL\n");
    }

    printf("\n\n");
    return 0;
}

rtems_task task_pci_test(rtems_task_argument argument)
{
    int ret;
    struct timespec t0, t1;
    pci_test_context_t * context = &pci_test_context;

    /* Init test context */
    context->task_stop_trigger = 0;
    ret = pci_test_init(context, PCI_TRANSFER_SIZE, 
            pci_buf_data_start, NULL, pci_buf_chanbd_start, pci_buf_databd_start, 
            PCI_DMA_DATA_DESCRIPTORS, PCI_BLOCK_SIZE, dma_isr);
    if (ret != 0){
        printf("PCI TASK: Failure when initializing test context: %d\n", ret);
        context->stop_trigger = 1;
    }

    /* Get task id */
    rtems_task_ident(RTEMS_SELF, RTEMS_SEARCH_ALL_NODES, &context->task_id);

    /* Create timeout semaphore */
    if (rtems_semaphore_create(rtems_build_name('S', 'E', '1', '0'), 0,
                RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | \
                RTEMS_NO_INHERIT_PRIORITY | RTEMS_LOCAL | \
                RTEMS_NO_PRIORITY_CEILING, 0, &context->timeout_sem) != RTEMS_SUCCESSFUL){
        printf("PCI TASK: Failure creating semaphore\n");
        rtems_task_delete( RTEMS_SELF );
    }

    /* Wait until test start */
    while (context->start_trigger == 0){
        rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }

    /* Start Pci test */
    printf("PCI TASK: Starting PCI test\n");
    rtems_clock_get_uptime(&t0);
    while (context->stop_trigger == 0){
        ret = pci_test_step(context);
        if (ret != 0){
            printf("PCI TASK: Failure on test step: %d\n", ret);
            context->stop_trigger = 1;
        }
    }

    /* Stop time */
    printf("PCI TASK: Stopping PCI test\n");
    rtems_clock_get_uptime(&t1);

    /* Save results */
    context->bytes_sent = (context->transfer_completed * context->data_size) +
        (context->transfer_idx * context->transfer_size);
    context->tstart = t0; 
    context->tstop = t1; 

    /* End context */
    ret = pci_test_end(context);
    if (ret != 0){
        printf("PCI TASK: Failure on test context end: %d\n", ret);
    }
    context->finished = 1;

    /* Delete task */
    while(context->task_stop_trigger == 0){
        rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }
    rtems_task_delete( RTEMS_SELF );
}

int pci_test_print_results( void )
{
    pci_test_context_t * context = &pci_test_context;
    while(context->finished == 0){
        rtems_task_wake_after(RTEMS_YIELD_PROCESSOR);
    }
    pci_test_results(context);
    return 0;
}

int pci_test_start( void )
{
    pci_test_context_t * context = &pci_test_context;
    if (context->start_trigger == 0){
        context->start_trigger = 1;
        return 0;
    }else{
        return -1;
    }
}

int pci_test_stop( void )
{
    pci_test_context_t * context = &pci_test_context;
    if (context->stop_trigger == 0){
        context->stop_trigger = 1;
        return 0;
    }else{
        return -1;
    }
}

int pci_test_task_stop( void )
{
    pci_test_context_t * context = &pci_test_context;
    if (context->task_stop_trigger == 0){
        context->task_stop_trigger = 1;
        return 0;
    }else{
        return -1;
    }
}
