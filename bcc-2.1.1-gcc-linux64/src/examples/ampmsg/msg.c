#include <bcc/bcc.h>

#include "msg.h"

void port_init(struct msgport *port)
{
        slist_init(&port->msgs);
        mutex_init(&port->lock);
        port->ipi = 0;
        port->cpuid = bcc_get_cpuid();
}

void port_set_ipi(struct msgport *port, int ipi)
{
        port->ipi = ipi;
}

int port_hasmsg(struct msgport *port)
{
        int empty;

        mutex_lock(&port->lock);
        empty = slist_is_empty(&port->msgs);
        mutex_unlock(&port->lock);

        return !empty;
}

struct msg *port_get(struct msgport *port)
{
        struct msg *msg;

        mutex_lock(&port->lock);
        msg = (struct msg *) slist_remhead(&port->msgs);
        mutex_unlock(&port->lock);

        return msg;
}

void msg_reply(struct msg *msg)
{
        struct msgport *port;

        port = msg->reply;
        msg->flags |= MSG_FLAG_REPLY;
        msg_send(port, msg);
}

void msg_send(struct msgport *port, struct msg *msg)
{
        mutex_lock(&port->lock);
        slist_addtail(&port->msgs, &msg->node);
        mutex_unlock(&port->lock);
        if (port->ipi) {
                bcc_send_interrupt(port->ipi, port->cpuid);
        }
}

void port_wait(struct msgport *port)
{
        while (1) {
                if (port_hasmsg(port)) {
                        break;
                }
                if (port->ipi) {
                        bcc_power_down();
                }
        }
}

