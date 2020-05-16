#include <bcc/bcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <msg.h>

#include <common.h>

/*
 * defines:
 * - AMP_COMM
 *   Shared memory area used during bootstrap
 * - AMP_NODES
 *   Total number of AMP nodes (BCC2 instances)
 * - NODE_INDEX
 *   Unique number for AMP instance, not necessarily tied to local cpuid
 */

struct comm {
        struct msgport *port[4];
};

volatile struct comm *sh = (struct comm *) AMP_COMM;

struct mutex mutex0;

#if NODE_INDEX == 0
static void node0_init(void)
{
        bcc_start_processor(1);
        while (NULL == sh->port[1]) {
        }
}
#endif

#if NODE_INDEX == 1
static void node1_init(void)
{
        struct msgport *p1;

        p1 = malloc(sizeof (*p1));
        if (NULL == p1) {
                printf("ERROR: could not allocate port\n");
                abort();
        }
        port_init(p1);
        /* Publish the port */
        sh->port[1] = p1;
}
#endif

void node0_main(struct msgport *port);
void node1_main(struct msgport *port);

int main(void)
{
#if NODE_INDEX == 0
        node0_init();
        node0_main(sh->port[1]);
#elif NODE_INDEX == 1
        node1_init();
        node1_main(sh->port[1]);
#else
        printf("ERROR: illegal NODE_INDEX=%d\n", NODE_INDEX);
#endif

        return 0;
}

void sleepus(uint32_t duration)
{
        uint32_t t0;
        uint32_t tdiff;
        t0 = bcc_timer_get_us();
        do {
                uint32_t t1;
                t1 = bcc_timer_get_us();
                tdiff = t1 - t0;
        } while (tdiff <= duration);
}

