/*
 * IRQMP timestamp
 *
 * This example demonstrates how to use the BCC interrupt timestamp API to
 * measure interrupt response time.
 *
 * EXAMPLE TRANSCRIPT
 *
 *   grmon3> run
 *   Number of timestamp register sets available: 2
 *   This example uses irqmp timestamp register set 0.
 *   This example uses gptimer1, subtimer 0.
 *   This example uses bus interrupt 6, which is mapped to irqmp interrupt 6.
 *   ass:    4080999 |  ack:  16 |  isr: 237
 *   ass:    5061586 |  ack:   6 |  isr:  88
 *   ass:    5953233 |  ack:   5 |  isr:  87
 *   ass:    6812886 |  ack:  10 |  isr:  92
 *
 * ass: counter value when interrupt was asserted.
 * ack: Number of cycles from ass until trap handling starts in CPU.
 * isr: Number of cycles from ass user ISR handler starts.
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#include <assert.h>
#include <stdio.h>

#include <bcc/bcc.h>
#include <bcc/timestamp.h>
#include <drv/timer.h>

#define dbg(...) if (0) { \
        printf("%s:%d: ", __func__, __LINE__); \
        printf(__VA_ARGS__); \
}

static volatile uint32_t mycnt;
struct timer_priv *tdev = NULL;

void myhandler(void *arg, int source)
{
        mycnt = bcc_timestamp_get_cnt();
        dbg("arg=%p, source=%d\n", arg, source);
}

void stampit(int stampnum, int intnum, int timersubnum)
{
        uint32_t v;
        void *sub;

        sub = timer_sub_open(tdev, timersubnum);
        assert(sub);

        /* disable timer, clear pending */
        timer_set_ctrl(sub, GPTIMER_CTRL_IP);
        timer_set_reload(sub, 0);

        bcc_timestamp_restart(stampnum, intnum);
        v = bcc_timestamp_status(stampnum);
        assert(v == 0);

        dbg("generate interrupt %d on timestamp %d\n", intnum, stampnum);

        timer_set_ctrl(sub, GPTIMER_CTRL_IE | GPTIMER_CTRL_LD | GPTIMER_CTRL_EN);

        do {
                v = bcc_timestamp_status(stampnum);
        } while (0 == (v & BCC_TIMESTAMP_ACK));

        /* Collect the numbers */
        uint32_t ass;
        uint32_t ack;
        uint32_t cnt;
        ass = bcc_timestamp_get_ass(stampnum);
        ack = bcc_timestamp_get_ack(stampnum);
        cnt = mycnt;
        printf("ass: %10lu |  ", cnt);
        printf("ack: %3lu |  ", ack - ass);
        printf("isr: %3lu\n", cnt - ass);

        timer_sub_close(tdev, timersubnum);
}

static int busintnum_to_irqmpintnum(int busintnum)
{
        int ret;

        ret = bcc_int_map_get(busintnum);
        if (ret < 0) {
                /* Interrupt map is not available, so it is one-to-one */
                ret = busintnum;
        }
        return ret;
}

int main(void)
{
        int ret;
        void *isr;
        const int stampnum = 0;
        int busintnum;
        int irqmpintnum;
        int timernum = 1;
        int timersubnum = 0;

        timer_autoinit();

        tdev = timer_open(timernum);
        assert(tdev);

        busintnum = timer_get_devreg(timernum)->interrupt;
        assert(busintnum);

        if (timer_get_cfg(tdev) & GPTIMER_CFG_SI) {
                busintnum += timersubnum;
        }
        irqmpintnum = busintnum_to_irqmpintnum(busintnum);

        timer_set_scaler_reload(tdev, 0xff);
        timer_set_scaler(tdev, 0xff);

        ret = bcc_timestamp_avail();
        printf("Number of timestamp register sets available: %d\n", ret);
        if (0 == ret) {
                puts("ERROR: This example requires at least one timestamp register set");
                return 1;
        }
        assert(stampnum < ret);

        dbg("set pil = 0xf\n");
        ret = bcc_set_pil(0xf);
        assert(0 == ret);

        printf("This example uses irqmp timestamp register set %d.\n",
            stampnum);
        printf("This example uses gptimer%d, subtimer %d.\n",
            timernum, timersubnum);
        printf("This example uses bus interrupt %d, which is mapped to irqmp interrupt %d.\n",
            busintnum, irqmpintnum);

        dbg("registering handler for irqmp interrupt %d\n", irqmpintnum);
        isr = bcc_isr_register(irqmpintnum, myhandler, (void *) stampnum);
        assert(NULL != isr);

        dbg("unmask interrupt source\n");
        ret = bcc_int_unmask(irqmpintnum);
        assert(BCC_OK == ret);

        dbg("set pil = 0x0\n");
        ret = bcc_set_pil(0x0);
        assert(0xf == ret);

        for (int i = 0; i < 10; i++) {
                stampit(stampnum, irqmpintnum, timersubnum);
        }

        return 0;
}

