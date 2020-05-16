/* This file represents a "work task" */

#include <bcc/bcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <msg.h>

#include "common.h"

static int process_msg(struct lmsg *m)
{
        printf("node1: ");
        printf("str=<%s>, ", m->str);
        printf("type=<%s>", LOG_LEVEL_TO_STR[m->level]);
        puts("");

        return m->level;
}

void node1_main(struct msgport *p1)
{
        sleepus(500000);
        printf("node1 is processing jobs arriving on port at %p\n\n", (void *) p1);
        while (1) {
                struct lmsg *m;
                int level;

                port_wait(p1);
                m = (struct lmsg *) port_get(p1);
                if (NULL == m) {
                        continue;
                }

                /* We own the message now.*/
                level = process_msg(m);
                msg_reply((struct msg *) m);
                /* We do not own the message anymore.*/

                if (level == LOG_LEVEL_POWERDOWN) {
                        break;
                }
        }

        printf("\nnode1 got a message with level=POWERDOWN, entering Power down mode... ZzZzz\n");
        while (1) {
                bcc_power_down();
        }
}

