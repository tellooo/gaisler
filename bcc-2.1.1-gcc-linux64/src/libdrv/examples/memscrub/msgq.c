/* Message queue for memscrub.
 * See msgq.h for more information.
 *
 * Author: Javier Jalle/Magnus Hjorth, Martin Åberg, Cobham Gaisler
 * Contact: support@gaisler.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msgq.h"
#include <drv/memscrub.h>
#include <drv/nelem.h>
#include "fifo.h"

enum { NBUFFERS = 32 };

struct msgq_priv {
  struct fifo fifo;
  struct memscrub_message msg[NBUFFERS];
  int overflow;
};
static struct msgq_priv thepriv;
static struct msgq_priv * msgqpriv;

int memscrub_msgq_push_none(void){
    struct memscrub_message msgin;

    msgin.type = MESSAGE_NONE;

    return memscrub_msgq_push(&msgin);
}

int memscrub_msgq_push_done(uint32_t ahbaccess, uint32_t ahbstatus, uint32_t status){
    UNUSED(ahbaccess);
    UNUSED(ahbstatus);
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
    UNUSED(status);
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

    ret = fifo_isfull(&priv->fifo);
    if (ret) {
            /* Overflow */
            priv->overflow++;
            return -1;
    }
    ret = fifo_wrptr(&priv->fifo);

    /* Copy */
    priv->msg[ret] = *msgin;
    fifo_put(&priv->fifo);

    return 0;
}

int memscrub_msgq_init()
{
    struct msgq_priv *priv;

    /* Allocate priv struct */
    priv = &thepriv;
    msgqpriv = priv;

    /* Default options */
    memset(priv,0,sizeof(*priv));

    fifo_init(&priv->fifo, NBUFFERS);

    return 0;
}

int memscrub_msgq_pop(int block, struct memscrub_message *msgout)
{
    struct msgq_priv *priv = msgqpriv;
    int ret;

    /* Check msgout */
    if (msgout == NULL){
        return -1;
    }

    msgout->type = MESSAGE_NONE;

    /* Check priv */
    if (priv == NULL){
        return -1;
    }

    do {
            /* peek */
            ret = fifo_rdptr(&priv->fifo);
            if (ret < 0 && !block) {
                return 0;
            }
    } while (ret < 0);

    /* copy */
    *msgout = priv->msg[ret];

    /* dequeue */
    fifo_get(&priv->fifo, NULL);

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

