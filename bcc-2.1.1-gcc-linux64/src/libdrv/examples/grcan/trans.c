/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * GRCAN driver example
 *
 * Two "tasks" are communicating. TX task sends data to RX task which
 * verify the data content. The RX task and the TX task collect statistics
 * which is printed periodically by a status task.  which calls
 * can_print_stats().
 *
 * In order for the example to work an external board 
 * responding to the messages sent is needed.
 * The example below expects the messages to be sent back
 * with the ID-field decremented once, all other data in 
 * message must be unmodified.
 *
 * The RX task may indicate dropped messages if the external
 * board doesn't send back all sent messages in time.
 * 
 * A software loopback (LOOP) task is also started on the second CAN device.
 * This task will just send back any CAN message is receives. It is useful for
 * testing GRCAN on a system with two GRCAN cores.
 *
 * Based on the RTEMS-5 GRCAN example by Cobham Gaisler AB, 2018
 * Daniel Hellström
 * Martin Åberg
 *
 */
/*
 * NOTE: When running this example on GR740, the following have to be configured:
 * - CAN pin multiplexing
 * - Enable GRCAN clock
 *
 * This can be done in GRMON using the following commands before running the
 * example:
 *   grmon3> wmem 0xffa0b000 0x000ffc3c
 *   grmon3> grcg enable 5
 *
 * In addition, when using GRCAN on the GR-CPCI-GR740 development board, the
 * PCB signal matrix has to be configured correspondingly:
 * - JP11 jumpers 15, 16, 21 and 22 shall be in the "BC" position.
 *
 * See the "GR-CPCI-GR740 Quick Start Guide" for more information.
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <drv/grcan.h>
#include <drv/timer.h>
#include <drv/gr716/grcan.h>
#include <drv/gr716/timer.h>

/*
 * 0: scan devices using ambapp
 * 1: use GR716 static driver tables
 */
#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

/* Select CAN core to be used in sample application.
 *  - 0                        (First CAN core)
 *  - 1                        (Second CAN core)
 *  - etc...
 */
#ifndef GRCAN_DEVICE_NUMBER
 #define GRCAN_DEVICE_NUMBER 0
#endif
/*#define GRCAN_DEVICE_NUMBER 1*/

/* If RX_MESSAGES_CHANGED if defined it is assumed that the
 * received messages has been changed from the transmitted
 * message. In that case it is assumed that the ID field of
 * each message has been decremented once.
 *
 * This option is useful when the messages are looped on an
 * external CAN board. CAN is not designed to receive the
 * exact same message as is beeing transmitted:
 *  WE SEND -> CAN_BUS -> External CAN Board changes the ID 
 *  field -> CAN_BUS -> WE RECEIVE and verify message
 */
/* #define RX_MESSAGES_CHANGED_ID
 * #define RX_MESSAGES_CHANGED_DATA
 */

/* Define this to get more statistics printed to console */
#undef PRINT_MORE_STATS

/* CAN Channel select */
enum {
	CAN_CHAN_SEL_A,
	CAN_CHAN_SEL_B,
	CAN_CHAN_SEL_NUM
};
int can_chan_sel = CAN_CHAN_SEL_A;	/* Default to channel A */
#if 1
int loop_chan_sel = CAN_CHAN_SEL_A;
#else
int loop_chan_sel = CAN_CHAN_SEL_B;
#endif

#define UNUSED(var) ((void) var)

/* CAN routines */
int can_init(int devno, int chan_sel);
void can_start(void);
void can_print_stats(void);

/* Loopback routines */
int loop_init(int devno, int chan_sel);
void loop_start(void);

struct timer_priv *tdev = NULL;
void *tsub = NULL;

#include <bcc/bcc_param.h>
/* ========================================================= 
   initialisation */

static const uint32_t SYNC_TIMEOUT_MS = 16;
int open_the_timer(int timernum, int timersubnum)
{
	tdev = timer_open(timernum);
	assert(tdev);

	{
		/* Get scaler reload from GPTIMER0, assuming to be 1 Hz */
		volatile struct gptimer_regs *gptimer0_regs = (void *) __bcc_timer_handle;
		uint32_t reload = gptimer0_regs->scaler_reload;
		timer_set_scaler_reload(tdev, reload);
		timer_set_scaler(tdev, reload);
	}
	tsub = timer_sub_open(tdev, timersubnum);
	assert(tsub);
	printf("This example uses gptimer%d, subtimer %d.\n",
	    timernum, timersubnum);
	timer_set_ctrl(tsub, 0);
	timer_set_reload(tsub, SYNC_TIMEOUT_MS * 1000);
	return 0;
}

int syncit(void)
{
	while (GPTIMER_CTRL_EN & timer_get_ctrl(tsub));
	timer_set_ctrl(tsub, GPTIMER_CTRL_EN | GPTIMER_CTRL_LD);
	return 0;
}

int main(void)
{
	int loop_ret;
	int ngrcan = 0;

	printf("******** Initializing CAN test ********\n");

	if (CFG_TARGET_GR716) {
		grcan_init(GR716_GRCAN_DRV_ALL);
		timer_init(GR716_TIMER_DRV_ALL);
	} else {
		grcan_autoinit();
		timer_autoinit();
	}

	open_the_timer(1, 0);

	ngrcan = grcan_dev_count();
	if (ngrcan < 1) {
		printf("Found no GRCAN cores, aborting\n");
		exit(0);
	}
	printf("Found %d GRCAN cores\n", ngrcan);

	/* Initialize RX/TX */
	if (can_init(GRCAN_DEVICE_NUMBER, can_chan_sel)) {
		printf("CAN INITIALIZATION FAILED, aborting\n");
		exit(1);
	}

	/* Initialize Loopback */
	loop_ret = loop_init(!GRCAN_DEVICE_NUMBER, loop_chan_sel);
	if (loop_ret) {
		printf("INFO: LOOP INITIALIZATION FAILED. Loop task deactivated.\n");
	}

	can_start();

	return 0;
}

/* CAN Implementation */

int canrx_task(int argument);
int cantx_task(int argument);

/* File descriptors of /dev/grcan0 */
void *candev;

/* Print one CAN message to terminal */
void print_msg(int i, struct grcan_canmsg * msg);

/* Initializes the 8 CAN messages in the global variable 
 * "struct grcan_canmsg msgs[8]".
 */
void init_send_messages(void);

/* Verify content of CAN message 'msg' against msgs[index].
 * Returns what went wrong.
 */
int verify_msg(struct grcan_canmsg * msg, int index);

/* ========================================================= 
   initialisation */

static const struct grcan_timing CAN_TIMING = {
#if 0
	/* Set baud rate: 250k @ 30MHz */
	.scaler = 3,
	.ps1 = 8,
	.ps2 = 5,
	.rsj = 1,
	.bpr = 1,
#elif 0
	/* Set baud rate: 250k @ 40MHz */
	.scaler = 7,
	.ps1 = 0xf,
	.ps2 = 0x3,
	.rsj = 0x1,
	.bpr = 0,
#else
	/* Set baud rate: 250k @ 250MHz */
	/* according to calc_can_btrs */
	.scaler = 0x27,
	.ps1 = 0xf,
	.ps2 = 0x8,
	.rsj = 0x1,
	.bpr = 0x0,
#endif
};

static const struct grcan_selection CAN_CHAN_SEL[CAN_CHAN_SEL_NUM] = {
	{
		/* Channel A */
		.selection = 0,
		.enable0 = 0,
		.enable1 = 1,
	},
	{
		/* Channel B */
		.selection = 1,
		.enable0 = 1,
		.enable1 = 0,
	},
};

static int can_init_dev(int devno, int chan_sel, void **dev_new);

int loop_task1(int argument);

struct loop_stats {
	unsigned int received;
	unsigned int sent;
	unsigned int rx_err;
};
struct loop_ctx {
	struct loop_stats stats;
	void *dev;
	struct grcan_canmsg msg[50];
};

struct loop_ctx _loop_ctx;

/* Loopback task */
int loop_task1(int arg)
{
	int rxmsgs;
	int txmsgs;
	int rxbytes;
	int txbytes;
	struct loop_ctx *ctx;

	ctx = (struct loop_ctx *) arg;
	if (!ctx->dev) {
		return 1;
	}

	//iprintf("%s: Entering CAN Loop-Back loop\n", __func__);

	int loopit = 1;
	while (loopit) {
		/* non-blocking read */
		rxbytes = grcan_read(ctx->dev, &ctx->msg[0], 40);
		if (rxbytes > 0) {
			//printf("%s: rxbytes=%d\n", __func__, rxbytes);
			rxmsgs = rxbytes;
			ctx->stats.received += rxmsgs;
			/* Just try send if possible */
			txbytes = grcan_write(ctx->dev, &ctx->msg[0], rxbytes);
			//printf("%s: txbytes=%d\n", __func__, txbytes);
			if (txbytes > 0) {
				txmsgs = txbytes;
				ctx->stats.sent += txmsgs;
			} else {
				grcan_stop(ctx->dev);
				grcan_start(ctx->dev);
			}
		//} else if ((rxbytes == -2) || (rxbytes == -4)) {
		} else if (rxbytes < 0) {
			iprintf("%s: Experienced RX error (%d) recovering with grcan_start()\n", __func__, rxbytes);
			/* OK to start since this task is the only user of ctx->dev. */
			grcan_stop(ctx->dev);
			grcan_start(ctx->dev);
			ctx->stats.rx_err++;
			//sleep(1);
			loopit = 0;
		} else {
			/* if in non-blocking mode we work with other stuff here */
			//iprintf("%s: RX blocking not working\n", __func__);
			//ctx->stats.rx_err++;
			//sleep(1);
			loopit = 0;
		}
	}
	return 0;
}


int loop_init(int devno, int chan_sel)
{
	printf("******** Initializing GRCAN loopback ********\n");

	return can_init_dev(devno, chan_sel, &_loop_ctx.dev);
}

/*
 * Open and bring up a can device in a sane configuration.
 *
 * This function does not change global variables, except via dev_new.
 */
static int can_init_dev(int devno, int chan_sel, void **dev_new)
{
	int ret;
	const struct grcan_selection *selection;
	void *dev;

	if (!dev_new) {
		return -1;
	}
	*dev_new = NULL;

	printf("Opening GRCAN device %d\n", devno);
	dev = grcan_open(devno);
	if (NULL == dev) {
		printf("Failed to open GRCAN device %d\n", devno);
		return -1;
	}

	/* Start GRCAN driver */

	/* Select CAN channel */
	if (chan_sel == CAN_CHAN_SEL_A) {
		selection = &CAN_CHAN_SEL[CAN_CHAN_SEL_A];
	} else {
		selection = &CAN_CHAN_SEL[CAN_CHAN_SEL_B];
	}

	/* Set up CAN driver:
	 *  ¤ baud rate
	 *  ¤ Channel
	 *  ¤ TX blocking, and wait for all data to be sent.
	 *  ¤ RX non-blocking depending on ONE_TASK mode
	 */
	/* Set baud */
	ret = grcan_set_btrs(dev, &CAN_TIMING);
	if (ret) {
		printf("grcan_set_btrs() failed: %d\n", ret);
		return -1;
	}
	ret = grcan_set_selection(dev, selection);
	if (ret) {
		printf("grcan_set_selection() failed: %d\n", ret);
		return -1;
	}

	ret = grcan_clr_stats(dev);
	if (ret) {
		printf("grcan_clr_stats() failed: %d\n", ret);
		return -1;
	}

	/* Start communication */
	ret = grcan_start(dev);
	if (ret) {
		printf("grcan_start() failed: %d\n", ret);
		return -1;
	}

	*dev_new = dev;

	return 0;
}

void can_start(void)
{
	int statdelay = 0;
	int slot = 0;
	const int NSLOTS = 4;

	syncit();
	while (1) {
		//printf("."); fflush(NULL);
		switch (slot) {
			case 0: {
				canrx_task(1);
			} break;
			case 1: {
				cantx_task(1);
			} break;
			case 2: {
				loop_task1((int) &_loop_ctx);
			} break;
			case 3: {
				if (0 == statdelay) {
					can_print_stats();
					statdelay = 2000 / (SYNC_TIMEOUT_MS * NSLOTS);
				} else {
					statdelay--;
				}
			} break;
		}
		slot++;
		if (NSLOTS <= slot) {
			slot = 0;
		}
		syncit();
	}
}

#define ID_GAISLER 0x2000

static struct grcan_canmsg msgs[8];

void init_send_messages(void)
{
	/* Send 1 STD Message */
	msgs[0].extended = 0;
	msgs[0].rtr = 0;
	msgs[0].unused = 0;
	msgs[0].id = 10;
	msgs[0].len = 4;
	msgs[0].data[0] = 0x2;
	msgs[0].data[1] = 0xc4;
	msgs[0].data[2] = 0x4f;
	msgs[0].data[3] = 0xf2;
	msgs[0].data[4] = 0x23;

	/* Send 3 EXT Message */
	msgs[1].extended = 1;
	msgs[1].rtr = 0;
	msgs[1].unused = 0;
	msgs[1].id = 10;
	msgs[1].len = 4 + 1;
	msgs[1].data[0] = 0x2;
	msgs[1].data[1] = 0xc4;
	msgs[1].data[2] = 0x4f;
	msgs[1].data[3] = 0xf2;
	msgs[1].data[4] = 0x23;
	msgs[1].data[5] = 0xa2;

	msgs[2].extended = 1;
	msgs[2].rtr = 0;
	msgs[2].unused = 0;
	msgs[2].id = 10 + 880;
	msgs[2].len = 8;
	msgs[2].data[0] = 0xaa;
	msgs[2].data[1] = 0xbb;
	msgs[2].data[2] = 0x11;
	msgs[2].data[3] = 0x22;
	msgs[2].data[4] = 'U';
	msgs[2].data[5] = 0x12;
	msgs[2].data[6] = 0xff;
	msgs[2].data[7] = 0x00;

	msgs[3].extended = 1;
	msgs[3].rtr = 0;
	msgs[3].unused = 0;
	msgs[3].id = 0xff | ID_GAISLER;
	msgs[3].len = 7;
	msgs[3].data[0] = 'G';
	msgs[3].data[1] = 'a';
	msgs[3].data[2] = 'i';
	msgs[3].data[3] = 's';
	msgs[3].data[4] = 'l';
	msgs[3].data[5] = 'e';
	msgs[3].data[6] = 'r';

	/* Send 1 STD Message */
	msgs[4].extended = 0;
	msgs[4].rtr = 0;
	msgs[4].unused = 0;
	msgs[4].id = 10;
	msgs[4].len = 4;
	msgs[4].data[0] = 0x2;
	msgs[4].data[1] = 0xc4;
	msgs[4].data[2] = 0x4f;
	msgs[4].data[3] = 0xf2;
	msgs[4].data[4] = 0x23;

	/* Send 3 EXT Message */
	msgs[5].extended = 1;
	msgs[5].rtr = 0;
	msgs[5].unused = 0;
	msgs[5].id = 10;
	msgs[5].len = 4 + 1;
	msgs[5].data[0] = 0x2;
	msgs[5].data[1] = 0xc4;
	msgs[5].data[2] = 0x4f;
	msgs[5].data[3] = 0xf2;
	msgs[5].data[4] = 0x23;
	msgs[5].data[5] = 0xa2;

	msgs[6].extended = 1;
	msgs[6].rtr = 0;
	msgs[6].unused = 0;
	msgs[6].id = 10 + 880;
	msgs[6].len = 8;
	msgs[6].data[0] = 0xaa;
	msgs[6].data[1] = 0xbb;
	msgs[6].data[2] = 0x11;
	msgs[6].data[3] = 0x22;
	msgs[6].data[4] = 'U';
	msgs[6].data[5] = 0x12;
	msgs[6].data[6] = 0xff;
	msgs[6].data[7] = 0x00;

	msgs[7].extended = 1;
	msgs[7].rtr = 0;
	msgs[7].unused = 0;
	msgs[7].id = 0xff | ID_GAISLER;
	msgs[7].len = 7;
	msgs[7].data[0] = 'G';
	msgs[7].data[1] = 'a';
	msgs[7].data[2] = 'i';
	msgs[7].data[3] = 's';
	msgs[7].data[4] = 'l';
	msgs[7].data[5] = 'e';
	msgs[7].data[6] = 'r';
}

/* Verify content of a CAN message */
int verify_msg(struct grcan_canmsg * msg, int index)
{
	int i;
	struct grcan_canmsg *src = &msgs[index];

	if (
		(msg->extended && !src->extended) ||
		(!msg->extended && src->extended)
	) {
		printf(
			"Expected extended=%d but got extended=%d\n",
			src->extended,
			msg->extended
		);
		return -1;
	}

	if (msg->rtr != src->rtr) {
		/* Remote Transmission Request does not match */
		return -2;
	}
#ifdef RX_MESSAGES_CHANGED_ID
	/* Decremented the ID once */
	if (msg->id != (src->id - 1)) {
#else
	if (msg->id != src->id) {
#endif
		printf("Expected id=0x%x but got id=0x%x\n", src->id, msg->id);
		return -3;
	}

	if (msg->len != src->len) {
		printf("Expected len=%d but got len=%d\n", src->len, msg->len);
		return -4;
	}
	for (i = 0; i < msg->len; i++) {
#ifdef RX_MESSAGES_CHANGED_DATA
		if (msg->data[i] != (src->data[i] + 1))
#else
		if (msg->data[i] != src->data[i])
#endif
			return -5 - i;
	}
	return 0;
}

/* Staticstics */
static volatile int rxpkts = 0, txpkts = 0;
/*
 * rx_syncs: Number of times an unexpected packet is received.
 * rx_errors: Number of packets not matching a packet known by the example.
 */
static volatile int rx_syncs = 0, rx_errors = 0;

/* RX Task */
int canrx_task(int unused)
{
	UNUSED(unused);
	struct grcan_canmsg rxmsgs[10];
	static int index = 0;

	while (1) {
		int cnt;
		cnt = grcan_read(candev, rxmsgs, 10);
		if (cnt < 0) {
			printf("%s: got BUSOFF, doing error recovery with grcan_start()\n", __func__);
			grcan_stop(candev);
			grcan_start(candev);
			return 1;
		} else if (cnt == 0) {
			return 0;
		}

		/* Statistics */
		rxpkts += cnt;

		/* For every message received we compare the content against
		 * expected content.
		 *
		 * If a message have been dropped we synchronize with the
		 * message stream to avoid getting multiple errors from one
		 * dropped message.
		 *
		 */
		for (int i = 0; i < (int) (cnt); i++) {
			int error;
			//printf("cnt=%d\n", cnt);
			//printf(".");
			//fflush(NULL);
			error = verify_msg(&rxmsgs[i], index);
			if (error) {
				printf("Message rx error: %d, index: %d\n", error, index);

				/* Print message */
				print_msg(0, &rxmsgs[i]);

				/* Try to sync if one has been lost */
				int e = 0;
				for (int j = 0; j < 4; j++) {
					if (!verify_msg(&rxmsgs[i], j)) {
						printf ("Synced from message %d to %d\n", index, j);
						rx_syncs++;
						index = j;
						e = 1;
						break;
					}
				}
				if (e != 1) {
					rx_errors++;
				}
			}
			index++;
			if (index > 3) {
				index = 0;
			}
		}
	}

	return 0;
}

/* TX Task */
int cantx_task(int unused)
{
	UNUSED(unused);
	static int ofs = 0;
#ifdef WAIT_AFTER_256_MSGS
	static int last = 0;
#endif

	{
		int cnt;
		/*
		 * Non-blocking transmit request. Returns when all messages
		 * requested have been scheduled or when no more messages can
		 * be scheduled.  (May not yet have been sent on bus at return.)
		 */
		cnt = grcan_write(candev, &msgs[ofs], 4);
		if (cnt > 0) {
			/* Increment statistics */
			txpkts += cnt;
			ofs += cnt;
			if (ofs > 3) {
				ofs -= 4;
			}
		} else if (cnt == 0) {
			;
		} else {
			printf("TX CAN TASK: write failed: %d\n", cnt);
			/* NOTE: RX task is responsible for handling the error */
			return 1;
		}

#ifdef WAIT_AFTER_256_MSGS
		/* Wait a bit after each 256 messages */
		if ((txpkts & 0x100) && !(last & 0x100)) {
			wake_after(4);
		}
		last = txpkts;
#endif
	}
	return 0;
}


int can_init(int devno, int chan_sel)
{
	printf("******** Initializing GRCAN test ********\n");

	printf("Initing messages\n");
	init_send_messages();

	printf
	    ("************** MESSAGES THAT WILL BE TRANSMITTED *************\n");
	print_msg(1, &msgs[0]);
	print_msg(2, &msgs[1]);
	print_msg(3, &msgs[2]);
	print_msg(4, &msgs[3]);
	printf
	    ("**************************************************************\n");

	printf
	    ("******************* Start of transmission ********************\n");

	txpkts = 0;

	return can_init_dev(devno, chan_sel, &candev);
}

/* CAN HELP DEBUG FUNCTIONS */
char *msgstr_type[2] = { "STD", "EXT" };
char *msgstr_rtr[2] = { "", " RTR" };

/* PRINT A CAN MESSAGE FROM DATA STRUCTURE */
void print_msg(int i, struct grcan_canmsg * msg)
{
	int j;
	char data_str_buf[64];
	int ofs;

	if (!msg)
		return;

	if (i > 0) {
		printf("MSG[%d]: %s%s length: %d, id: 0x%x\n", i,
		       msgstr_type[(int)msg->extended],
		       msgstr_rtr[(int)msg->rtr], msg->len, msg->id);
		/* print data */
		if (msg->len > 0) {
			ofs = sprintf(data_str_buf, "MSGDATA[%d]: ", i);
			for (j = 0; j < msg->len; j++) {
				ofs +=
				    sprintf(data_str_buf + ofs, "0x%02x ",
					    msg->data[j]);
			}
			printf("%s  ", data_str_buf);
			ofs = 0;
			for (j = 0; j < msg->len; j++) {
				if (isalnum(msg->data[j]))
					ofs +=
					    sprintf(data_str_buf + ofs, "%c",
						    msg->data[j]);
				else
					ofs += sprintf(data_str_buf + ofs, ".");
			}
			printf("%s\n", data_str_buf);
		}
	} else {
		printf("MSG: %s%s length: %d, id: 0x%x\n",
		       msgstr_type[(int)msg->extended],
		       msgstr_rtr[(int)msg->rtr], msg->len, msg->id);
		/* print data */
		if (msg->len > 0) {
			ofs = sprintf(data_str_buf, "MSGDATA: ");
			for (j = 0; j < msg->len; j++) {
				ofs +=
				    sprintf(data_str_buf + ofs, "0x%02x ",
					    msg->data[j]);
			}
			printf("%s  ", data_str_buf);
			ofs = 0;
			for (j = 0; j < msg->len; j++) {
				if (isalnum(msg->data[j]))
					ofs +=
					    sprintf(data_str_buf + ofs, "%c",
						    msg->data[j]);
				else
					ofs += sprintf(data_str_buf + ofs, ".");
			}
			printf("%s\n", data_str_buf);
		}
	}
}

/* Print statistics gathered by RX and TX tasks, also
 * print statistics from driver.
 */
void can_print_stats(void)
{
	struct grcan_stats stats;
	static int cnt = 0;
	int ret;

	/* Get stats from GRCAN driver to print */
	ret = grcan_get_stats(candev, &stats);
	if (ret) {
		printf("grcan_get_stats() failed: %d (continue anyway)\n", ret);
	} else {
		/* Got stats from driver */

#ifdef PRINT_MORE_STATS
		/* Print extra stats */
		printf("CAN PASSV:   %7d\n", stats.passive_cnt);
		printf("CAN OVERRUN: %7d\n", stats.overrun_cnt);
		printf("CAN TXLOSS:  %7d\n", stats.txloss_cnt);
#endif
		if (stats.busoff_cnt) {
			printf("CAN BUSOFF:  %7d\n", stats.busoff_cnt);
		}
		if (stats.ahberr_cnt) {
			printf("CAN AHB:     %7d\n", stats.ahberr_cnt);
		}
		printf("CAN INTS:    %7d\n", stats.ints);
	}
	printf("CAN RXPKTS:  %7d\n", rxpkts);

	/* Print only number of RX syncs every tenth time */
	if (cnt++ >= 10) {
		cnt = 0;
		printf("CAN RXSYNCS: %7d\n", rx_syncs);
		if (rx_errors > 0) {
			printf("CAN RXERRORS: %7d\n", rx_errors);
		}
	}

	printf("CAN TXPKTS:  %7d  (TX-RX: %7d)\n", txpkts, txpkts-rxpkts);
}

