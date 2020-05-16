#ifndef _MSG_H_
#define _MSG_H_

/*
 * A message is sent to a port.
 * Messages appear in FIFO order on the port.
 * The port owner can sleep on the port to be non-empty.
 * A message port is protected with a mutexe.
 * Many processors can send messages simultaneously to a message port.
 * When a message has been sent to a port, the message is owned by the port
 * owner.
 * A message can be replied back to the original sender. Then the original
 * sender owns the message again.
 * A message can not be sent from ISR context.
 */

#include <mutex.h>
#include <slist.h>

struct msgport {
        struct slist msgs;
        struct mutex lock;
        int ipi;
        int cpuid;
};

enum {
        MSG_FLAG_REPLY  = 0x01,
};

struct msg {
        struct slistnode node;
        struct msgport *reply;
        unsigned int flags;
};

void port_init(struct msgport *port);

/*
 * Send a message to a port. After the mssage is sent it shall not be accessed
 * by the sender.
 *
 * If the port owner has set port->ipi != then then an inter-processor
 * interrupt will be sent.
 */
void msg_send(struct msgport *port, struct msg *msg);

/* Reply a message back to the port set in the msg->reply field. */
void msg_reply(struct msg *msg);

/*
 * returns true if there is a message available, else false
 * never blocks
 */
int port_hasmsg(struct msgport *port);

/*
 * returns NULL if no msg in port, otherwise removes the first message and
 * returns it.
 *
 * This function will never block
 */
struct msg *port_get(struct msgport *port);

/*
 * Wait until there is at least one message on the port. This function can
 * block.
 *
 * If the port->ipi value is != 0 then power-down is entered while waiting.
 *
 * NOTE: there may be more than one message on the port. Take them off with
 * port_get().
 */
void port_wait(struct msgport *port);

#endif

