/* RTEMS message queue for the memscrub. 
 * 
 * Author: Javier Jalle/Magnus Hjorth, Cobham Gaisler
 * Contact: support@gaisler.com
 */

/*-----------------------------------------------------------------------
 * Operation:
 *
 * An RTEMS message queue is used to communicate status information
 * from the driver's interrupt handler to the user. Depending on mode,
 * different amount of messages will be sent. The driver ignores errors 
 * resulting from the queue being full, therefore the user is not 
 * required to read out the queue. The application needs to provide 
 * room for the message queue via the configuration table.
 */

#ifndef MEMSCRUB_MSGQ_H_INCLUDED
#define MEMSCRUB_MSGQ_H_INCLUDED

#include <drvmgr/drvmgr.h>
#include <grlib/ambapp_bus.h>

/* Resources required for driver message queue */
#define MEMSCRUB_MAXIMUM_MESSAGE_QUEUES 1
#define MEMSCRUB_MESSAGE_BUFFER_MEMORY CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(8,sizeof(struct memscrub_message))

/* Message structure */
typedef enum {
    MESSAGE_NONE, /* None */ 
    MESSAGE_DONE, /* Run done */
    MESSAGE_ERROR, /* Error detected */
    MESSAGE_REGEN, /* Starting regeneration */
    MESSAGE_CUSTOM /* Custom */
} message_type_t;
struct memscrub_message {
    message_type_t type; 
    union{
        struct {
            int ce_count;
            int ue_count;
            int errtype;
            uint32_t addr;
            int master;
            int hwrite;
            int hsize;
        } error;
        struct {
            int run_count;
            int block_count;
        } done;
        struct {
            uint32_t word0;
            uint32_t word1;
        } custom;
    } info;
};

/* Init message queue struct */
int memscrub_msgq_init();

/* Pop a message from the driver's message queue and return it in msgout.
 * If no message is available, msgout->type is set to none
 * If block=1, the calling task is blocked until a message is available */
int memscrub_msgq_pop(int block, struct memscrub_message *msg);

/* Push specficic messages */
int memscrub_msgq_push_none(void);
int memscrub_msgq_push_done(uint32_t ahbaccess, uint32_t ahbstatus, uint32_t status);
int memscrub_msgq_push_error(uint32_t ahbaccess, uint32_t ahbstatus, uint32_t status);
int memscrub_msgq_push_regen(void);
int memscrub_msgq_push_custom(uint32_t word0, uint32_t word1);

/* Push a message */
int memscrub_msgq_push(struct memscrub_message *msg);

/* Print a message */
int memscrub_msgq_print(struct memscrub_message *msg);

/* Print a debug/status message to the console */
int memscrub_msgq_status(void);

/* Read out the total number of correctable/uncorrectable errors encountered */
int memscrubr_get_totals(int totals[2]);

#endif
