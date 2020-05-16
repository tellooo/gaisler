#include <bcc/bcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <msg.h>

#include "common.h"

#define NMSG 23

struct lmsg myjobs[NMSG];
char jobstr[NMSG][120];

/* Block wait for a number of messages */
static void getback(struct msgport *p, int n)
{
        int i;

        i = 0;
        while (i < n) {
                struct msg *m;

                port_wait(p);
                m = port_get(p);
                if (m) {
                        i++;
                }
        }
}

void node0_main(struct msgport *p1)
{
        /* Reply port for node0 */
        struct msgport *rp;
        char bench[120];
        struct lmsg benchjob;
        uint32_t t0, t1;
        int i;

        printf("--- EXAMPLE BEGIN --- \n");
        sleepus(1000000);
        rp = malloc(sizeof (*rp));
        if (NULL == rp) {
                printf("ERROR: could not allocate reply port\n");
                abort();
        }
        port_init(rp);

        for (i = 0; i < NMSG; i++) {
                myjobs[i].msg.reply = rp;
                myjobs[i].level = i % 4;
                myjobs[i].str = &jobstr[i][0];
                sprintf(myjobs[i].str, "this is job %3d (%p) dispatched from node0", i, (void *) &myjobs[i]);
        }
        benchjob.msg.reply = rp;
        benchjob.level = LOG_LEVEL_INFO;
        benchjob.str = "Benchmark report job";

        t0 = bcc_timer_get_us();
        for (i = 0; i < NMSG; i++) {
                msg_send(p1, (struct msg *) &myjobs[i]);
        }
        t1 = bcc_timer_get_us();

        getback(rp, NMSG);

        sprintf(&bench[0], "it took node0 %u us to dispatch %d jobs asynchronously", (unsigned int) (t1-t0), NMSG);
        benchjob.str = &bench[0];

        msg_send(p1, (struct msg *) &benchjob);
        getback(rp, 1);


        /* Now only one outstanding message at a time. */
        t0 = bcc_timer_get_us();
        for (i = 0; i < NMSG; i++) {
                msg_send(p1, (struct msg *) &myjobs[i]);
                getback(rp, 1);
        }
        t1 = bcc_timer_get_us();

        sprintf(&bench[0], "it took node0 %u us to dispatch (and wait for) %d jobs synchronously", (unsigned int) (t1-t0), NMSG);

        msg_send(p1, (struct msg *) &benchjob);
        getback(rp, 1);

        sprintf(&bench[0], "this is a power-down message from node0 to port %p", (void *) p1);
        benchjob.level = LOG_LEVEL_POWERDOWN;
        msg_send(p1, (struct msg *) &benchjob);
        getback(rp, 1);

        sleepus(500000);
        printf("\n--- EXAMPLE END --- \n");
}

