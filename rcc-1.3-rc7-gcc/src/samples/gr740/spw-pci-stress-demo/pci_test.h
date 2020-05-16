#ifndef __PCI_TEST_H__
#define __PCI_TEST_H__
#include <grlib/grpci2.h>

#define PCI_TRANSFER_SIZE (128*256*1024)
#define PCI_BLOCK_SIZE (256*1024)
#define PCI_DMA_DATA_DESCRIPTORS 128
#define PCI_TIMEOUT 200000 /* Timeout after I ticks. */

int pci_test_start(void);
int pci_test_stop(void);
int pci_test_task_stop(void);
int pci_test_print_results(void);
rtems_task task_pci_test(rtems_task_argument argument);
#endif
