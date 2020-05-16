
/* RTEMS message queue for memscrub.
 * See msgq.h for more information.
 *
 * Author: Javier Jalle/Magnus Hjorth, Cobham Gaisler
 * Contact: support@gaisler.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msgq.h"
#include <grlib/memscrub.h>

struct msgq_priv {
  rtems_id msgq;
  int overflow;
};
static struct msgq_priv * msgqpriv;

int memscrub_msgq_push_none(void){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_NONE;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push_done(uint32_t ahbaccess, uint32_t ahbstatus, uint32_t status){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_DONE;
    msgin.info.done.run_count = (status & STAT_RUNCOUNT) >> STAT_RUNCOUNT_BIT;
    msgin.info.done.block_count = (status & STAT_BLKCOUNT) >> STAT_BLKCOUNT_BIT;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push_regen(void){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_REGEN;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push_custom(uint32_t word0, uint32_t word1){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_CUSTOM;
    msgin.info.custom.word0=word0;
    msgin.info.custom.word1=word1;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push_error(uint32_t ahbaccess, uint32_t ahbstatus, uint32_t status){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_ERROR;
    msgin.info.error.ce_count = (ahbstatus & AHBS_CECNT) >> AHBS_CECNT_BIT;
    msgin.info.error.ue_count = (ahbstatus & AHBS_UECNT) >> AHBS_UECNT_BIT;
    msgin.info.error.errtype = (ahbstatus & (AHBS_NE|AHBS_SBC|AHBS_SEC|AHBS_CE));
    msgin.info.error.addr = (ahbaccess);
    msgin.info.error.master = (ahbstatus & AHBS_HM) >> AHBS_HM_BIT;
    msgin.info.error.hwrite = (ahbstatus & AHBS_HW) >> AHBS_HW_BIT;
    msgin.info.error.hsize = (ahbstatus & AHBS_HS) >> AHBS_HS_BIT;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push(struct memscrub_message *msgin){
    struct msgq_priv * priv = msgqpriv;
    int ret;

    if(priv == NULL){
        return -1;
    }

    if(msgin == NULL){
        return -1;
    }

    /* Send message */
    ret = rtems_message_queue_send(priv->msgq,msgin,sizeof(struct memscrub_message));
    if (ret!=RTEMS_SUCCESSFUL){
        if (ret==RTEMS_TOO_MANY){
            /* Overflow */
            priv->overflow++;
            return -1;
        }else{
            rtems_fatal_error_occurred(ret);
            return -1;
        }
    }
    return 0;
}

int memscrub_msgq_init()
{
    struct msgq_priv *priv;
    int ret;

    /* Allocate priv struct */
    priv = malloc(sizeof(*priv));
    if (priv == NULL){
        return -1;
    }
    msgqpriv = priv;

    /* Default options */
    memset(priv,0,sizeof(*priv));

    /* Create message queue for ISR */
    ret = rtems_message_queue_create(
            rtems_build_name('S','C','R','B'), /* Name */
            8, /* Message count */
            sizeof(struct memscrub_message), /* Max message size */
            RTEMS_GLOBAL, /* Global message queue */
            &(priv->msgq) /* ID */
        );
    if (ret != RTEMS_SUCCESSFUL){
        rtems_fatal_error_occurred(ret);
        return -1;
    }

    return ret;
}

int memscrub_msgq_pop(int block, struct memscrub_message *msgout)
{
    struct msgq_priv *priv = msgqpriv;
    int ret;
    size_t msgsize;

    /* Check msgout */
    if (msgout == NULL){
        return -1;
    }

    msgout->type = MESSAGE_NONE;

    /* Check priv */
    if (priv == NULL){
        return -1;
    }

    ret = rtems_message_queue_receive(
            priv->msgq, 
            msgout, 
            &msgsize, 
            ( (block == 0)? RTEMS_NO_WAIT:RTEMS_WAIT), 
            RTEMS_NO_TIMEOUT);
    if ((ret == RTEMS_UNSATISFIED) && (block==0)){
        return 0;
    }
    if (ret != RTEMS_SUCCESSFUL){
        return -1;
    }

    return 0;
}

int memscrub_msgq_status(void){
    struct msgq_priv *priv = msgqpriv;
    uint32_t count;
    int ret;

    /* Check priv */
    if (priv == NULL){
        return -1;
    }

    ret = rtems_message_queue_get_number_pending(priv->msgq, &count);
    if (ret != RTEMS_SUCCESSFUL){
        return -1;
    }

    printf("### Message queue has %d pending messages.\n", (unsigned int) count);

    return 0;
}

int memscrub_msgq_print(struct memscrub_message * msg){
    if (msg == NULL){
        return -1;
    }
    switch (msg->type) {
        case MESSAGE_DONE: 
            printf("[R] Scrubber iteration done, runcount=%d, blockcount=%d\n",
                    msg->info.done.run_count,
                    msg->info.done.block_count);
            break;
        case MESSAGE_ERROR:
            printf("[R] %s detected, addr=%08x, %s, mst=%d, size=%d\n",
                    (msg->info.error.errtype & AHBS_CE)?"CE":"UE",
                    (unsigned int)msg->info.error.addr, 
                    msg->info.error.hwrite?"wr":"rd",
                    msg->info.error.master, 
                    (1<<msg->info.error.hsize));
            break;
        case MESSAGE_REGEN:
            printf("[R] Scrubber switched to regeneration mode\n");
            break;
        case MESSAGE_CUSTOM:
            printf("[R] Custom message: 0x%08x, 0x%08x\n",
                    (unsigned int) msg->info.custom.word0,
                    (unsigned int) msg->info.custom.word1);
            break;
        case MESSAGE_NONE:
        default:
            printf("[R] Empty message returned\n");
            break;
    }
    return 0;
}
