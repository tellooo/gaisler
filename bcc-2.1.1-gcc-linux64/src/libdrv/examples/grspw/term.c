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

/* Interactive example on how to use SpaceWire driver non-blocking interface. */

#include <stdio.h>
#include <string.h>
#include <drv/nelem.h>
#include <drv/grspw.h>
#include <drv/gr716/grspw.h>
#include "grspw_pkt_lib.h"
#include <bcc/capability.h>

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

//#define PRINT_GRSPW_RESET_CFG

int nospw = 0;

/* SpaceWire parameters */
#define SPW_PROT_ID 155

#define PKT_SIZE 32

struct grspw_device {
        struct grspw_dev dev;

	/* Test structures */
	struct grspw_config cfg;
	int run;

	/* RX and TX lists with packet buffers */
	struct grspw_list rx_list, tx_list, tx_buf_list;
	int rx_list_cnt, tx_list_cnt, tx_buf_list_cnt;
};

#define DEV(device) ((struct grspw_dev *)(device))

#define DEVS_MAX 6
static struct grspw_device devs[DEVS_MAX];

struct grspw_config dev_def_cfg = 
{
		.adrcfg =
		{
			.promiscuous = 1, /* Detect all packets */
			.def_addr = 32, /* updated bu dev_init() */
			.def_mask = 0,
			.dma_nacfg =
			{
				/* Since only one DMA Channel is used, only
				 * the default Address|Mask is used.
				 */
				{
					.node_en = 0,
					.node_addr = 0,
					.node_mask = 0,
				},
				{
					.node_en = 0,
					.node_addr = 0,
					.node_mask = 0,
				},
				{
					.node_en = 0,
					.node_addr = 0,
					.node_mask = 0,
				},
				{
					.node_en = 0,
					.node_addr = 0,
					.node_mask = 0,
				},
			},
		},
		.rmap_cfg = 0,		/* Disable RMAP */
		.rmap_dstkey = 0,	/* No RMAP DESTKEY needed when disabled */
		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
		.chan =
		{
			{
				.flags = DMAFLAG_NO_SPILL,
				.rxmaxlen = PKT_SIZE+4,
			},
			/* The other 3 DMA Channels are unused */
			
		},
};

/* SpaceWire Routing table entry */
struct route_entry {
	unsigned char dstadr[16];	/* 0 terminates array */
};

/* SpaceWire packet payload (data) content layout */
struct pkt_hdr {
	unsigned char addr;
	unsigned char protid;
	unsigned char port_src; /* port index of source */
	unsigned char resv2; /* Zero for now */
	unsigned int data[(PKT_SIZE-4)/4];
};

struct spwpkt {
	struct grspw_pkt p;
	unsigned long long data[PKT_SIZE/8+1]; /* 32 bytes of data - 4byte data-header (8 extra bytes to avoid truncated bad packets)*/
	unsigned long long hdr[2]; /* up to 16byte header (path address) */
};

/* All packet buffers used by application */
// #define PACKET_BUFFERS_PER_DEV (16+120)
#define PACKET_BUFFERS_PER_DEV (16+30)
struct spwpkt pkts[DEVS_MAX][PACKET_BUFFERS_PER_DEV];

/* Initialize packet header, and Data payload header */
void pkt_init_hdr(struct grspw_pkt *pkt, struct route_entry *route, int idx)
{
	int i;
	struct pkt_hdr *pkt_hdr = (struct pkt_hdr *)pkt->data;
	unsigned char *hdr = pkt->hdr;

	/* If path addressing we put non-first Destination Addresses in 
	 * header. route->dstadr[0] is always non-zero.
	 */
	i = 0;
	while ( route->dstadr[i+1] != 0 ) {
		hdr[i] = route->dstadr[i];
		i++;
	}
	/* Put last address in pkthdr->addr */
	pkt->hlen = i;
	pkt_hdr->addr = route->dstadr[i];
	pkt_hdr->protid = SPW_PROT_ID;
	pkt_hdr->port_src = idx;
	pkt_hdr->resv2 = 0;
}

#ifndef NELEM
#define NELEM(a) ((int) (sizeof(a) / sizeof(a[0])))
#endif

void init_pkts(void)
{
	struct spwpkt *pkt;
	int i, j;

	memset(&pkts[0][0], 0, sizeof(pkts));

	for (i = 0; i < DEVS_MAX; i++) {
		grspw_list_clr(&devs[i].rx_list);
		grspw_list_clr(&devs[i].tx_list);
		grspw_list_clr(&devs[i].tx_buf_list);
		devs[i].rx_list_cnt = 0;
		devs[i].tx_list_cnt = 0;
		devs[i].tx_buf_list_cnt = 0;
		for (j = 0, pkt = &pkts[i][0]; j < NELEM(pkts[0]); j++, pkt = &pkts[i][j]) {
			pkt->p.pkt_id = (i << 8)+ j; /* unused */
			pkt->p.data = &pkt->data[0];
			pkt->p.hdr = &pkt->hdr[0];
			if (j < (NELEM(pkts[0])-8)) {
				/* RX buffer */

				/* Add to device RX list */
				grspw_list_append(&devs[i].rx_list, &pkt->p);
				devs[i].rx_list_cnt++;
			} else {
				/* TX buffer */
				pkt->p.dlen = PKT_SIZE;
                                uint8_t *d = pkt->p.data;
				memset(d+4, i, PKT_SIZE-4);

				/* Add to device TX list */
				grspw_list_append(&devs[i].tx_buf_list, &pkt->p);
				devs[i].tx_buf_list_cnt++;
			}
		}
	}
}

int dev_init(int idx)
{
	struct grspw_device *devi = &devs[idx];
	struct grspw_dev    *dev = &devi ->dev;
	int i, ctrl, clkdiv;

	printf(" Initializing SpaceWire device %d\n", idx);

	memset(dev, 0, sizeof(*devi));

	dev->index = idx;
	dev->dh = grspw_open(idx);
	if (dev->dh == NULL) {
		printf("Failed to open GRSPW device %d\n", idx);
		return -1;
	}
	grspw_hw_support(dev->dh, &dev->hwsup);
#ifdef PRINT_GRSPW_RESET_CFG
	grspw_config_read(dev, &devi->cfg);
	printf("\n\n---- DEFAULT CONFIGURATION FROM DRIVER/HARDWARE ----\n");
	grspw_cfg_print(&dev->hwsup, &devi->cfg);
#endif
	devi->cfg = dev_def_cfg;
	devi->cfg.adrcfg.def_addr = 32 + idx;

	if (grspw_cfg_set(dev, &devi->cfg)) {
		grspw_close(dev->dh);
		return -1;
	}
#ifdef PRINT_GRSPW_RESET_CFG
	printf("\n\n---- APPLICATION CONFIGURATION ----\n");
	grspw_cfg_print(&dev->hwsup, &devi->cfg);
	printf("\n\n");
#endif

	/* This will result in an error if only one port available */
	if (dev->hwsup.nports < 2) {
		int port = 1;
		if ( grspw_port_ctrl(dev->dh, &port) == 0 ) {
			printf("Succeeded to select port1, however only one PORT on dev %d!\n", dev->index);
			return -1;
		}
	}

	/* Try to bring link up at clockdiv = 3 but do not touch
	 * start-up clockdivisor */
	clkdiv = -1;
	grspw_link_ctrl(dev->dh, NULL, NULL, &clkdiv);
	ctrl = LINKOPTS_ENABLE | LINKOPTS_AUTOSTART | LINKOPTS_START;
	clkdiv &= 0xff00;
	clkdiv |= 0x0003;
	grspw_link_ctrl(dev->dh, &ctrl, NULL, &clkdiv);
        #define GAISLER_SPW2_DMA        0x08a

	if ( (dev->hwsup.hw_version >> 16) == GAISLER_SPW2_DMA ) {
		printf(" NOTE: running on SPW-ROUTER DMA SpaceWire link (no link-state available)\n");
	} else {
		printf(" After Link Start:");
		printf(" %s\n", grspw_linkstate_to_name(grspw_link_state(dev->dh)));
        }
	devi->run = 0;

	for (i=0; i<dev->hwsup.ndma_chans; i++) {
		if (dev->dma[i])
			grspw_dma_stats_clr(dev->dma[i]);
	}
	
	grspw_list_clr(&devi->rx_list);
	grspw_list_clr(&devi->tx_list);
	grspw_list_clr(&devi->tx_buf_list);
	devi->rx_list_cnt = devi->tx_list_cnt = devi->tx_buf_list_cnt = 0;

	return 0;
}

int router_present = 0;
void commandloop(void);

int main(void)
{
        int i;

        if (CFG_TARGET_GR716) {
                grspw_init(GR716_GRSPW_DRV_ALL);
        } else {
                grspw_autoinit();
        }

	nospw = grspw_dev_count();
	if (nospw < 1) {
		printf("Found no SpaceWire cores, aborting\n");
		exit(0);
	}
        printf("Found %d SpaceWire cores\n", nospw);
	if (nospw > DEVS_MAX) {
		printf("Limiting to %d SpaceWire devices\n", DEVS_MAX);
		nospw = DEVS_MAX;
	}

	memset(devs, 0, sizeof(devs));
	for (i=0; i<nospw; i++) {
		if (dev_init(i)) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
                puts("");
		fflush(NULL);
	}

	/* Initialize GRSPW */
	init_pkts();

	printf("\n\nStarting SpW DMA channels\n");
	for (i = 0; i < nospw; i++) {
		printf("Starting GRSPW%d: ", i);
		fflush(NULL);
		if (grspw_start(DEV(&devs[i]))) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		printf("DMA Started Successfully\n");
	}

        commandloop();
	printf("\n\nEXAMPLE COMPLETED.\n\n");
        return 0;
}

int dma_process_tx(struct grspw_device *dev);
int dma_process_rx(struct grspw_device *dev);

struct lcmd {
        const char *name;
        const char *syntax;
        const char *desc;
        int (*func)(char **words, int word, int devno);
};

extern struct lcmd lcmds[];
static const char *PROMPT = "grspw> ";
enum {
        DEVNO_NONE      = -1,
        DEVNO_BAD       = -2,
};

static int func_speed(char **words, int word, int devno)
{
        int ret;
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

        if (word < 3) {
                printf("%s: command needs RUN parameter\n", words[0]);
                return 1;
        }
        unsigned int rundiv = strtoul(words[2], NULL, 0);
        unsigned int startdiv = 0;

        if (3 < word) {
                startdiv = strtoul(words[3], NULL, 0);
        }

        struct grspw_priv *dev = devs[devno].dev.dh;
        ret = grspw_set_clkdiv(dev, rundiv | startdiv << 8);
        if (ret) {
                printf("ERROR: grspw_link_ctrl() says '%d'\n", ret);
        }

        return 0;
}

static int func_link(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
	        struct grspw_link_state state;
		grspw_link_state_get(DEV(&devs[devno]), &state);
		printf(" GRSPW%d link/port setup:\n", devno);
		grspw_linkstate_print(&state);
		printf("\n");
                devno++;
        }

        return 0;
}

/* NOTE: could do the read in chunks */
static int func_r(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                printf(" grspw%d: --- receive begin ---\n", devno);
                dma_process_rx(&devs[devno]);
                printf(" grspw%d: --- receive end   ---\n", devno);
                devno++;
        }

        return 0;
}

static int func_x(char **words, int word, int devno)
{
        if (word < 2) {
                printf("%s: command need device number\n", words[0]);
                return 1;
        }

        if (devno < 0) {
                printf(" Invalid device number\n");
                return 1;
        }

	if (word > 17 || word < 3) {
		printf(" Invalid routing path\n");
		return 1;
	}
	struct grspw_pkt *pkt;
	struct route_entry route;
	memset(&route, 0, sizeof(route));
	for (int i = 2; i < word; i++) {
		route.dstadr[i - 2] = strtoul(words[i], NULL, 0);
        }

        pkt = grspw_list_remove_head(&devs[devno].tx_buf_list);
	if (pkt == NULL) {
		printf(" No free transmit buffers available\n");
		return 1;
	}
	devs[devno].tx_buf_list_cnt--;
	pkt_init_hdr(pkt, &route, devno);

	printf(" X%d: scheduling packet on GRSPW%d\n",
		devno, devno);
	/* Send packet by adding it to the tx_list */
	grspw_list_append(&devs[devno].tx_list, pkt);
	devs[devno].tx_list_cnt++;
        dma_process_tx(&devs[devno]);

        return 0;
}

/* NOTE: could do the read in chunks */
static int func_stats(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
	        struct grspw_stats stats;
	        int tx_hwcnt, rx_hwcnt;

                printf(" grspw%d: --- info begin ---\n", devno);
		/* Print statistics */
		printf("\n\n--- GRSPW%d Driver Stats ---\n", devno);
                /* NOTE: only on the first dma channel */
		rx_hwcnt = grspw_dma_rx_count(devs[devno].dev.dma[0]);
		tx_hwcnt = grspw_dma_tx_count(devs[devno].dev.dma[0]);
		printf(" DRVQ RX_HWCNT: %d\n", rx_hwcnt);
		printf(" DRVQ TX_HWCNT: %d\n", tx_hwcnt);

		grspw_stats_get(DEV(&devs[devno]), &stats);
		grspw_stats_print(DEV(&devs[devno]), &stats);
                printf(" grspw%d: --- info end   ---\n", devno);
                devno++;
        }

        return 0;
}

/* NOTE: could do the read in chunks */
static int func_info(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        int ntotal = nospw;

        if (devno == DEVNO_BAD) {
                printf(" Invalid device number\n");
                return 1;
        } else if (devno == DEVNO_NONE) {
                devno = 0;
        } else {
                ntotal = 1;
        }

        for (int i = 0; i < ntotal; i++) {
                printf(" grspw%d: --- info begin ---\n", devno);
		grspw_cfg_print(&devs[devno].dev.hwsup, NULL);
                printf(" grspw%d: --- info end   ---\n", devno);
                devno++;
        }

        return 0;
}

static int func_help(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        UNUSED(devno);
        for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                char buf[32];
                snprintf(buf, 32, "%s%s", cmd->name, cmd->syntax);
                buf[31] = '\0';
                printf(" %-22s - %s\n", buf, cmd->desc);
        }
        return 0;
}

int quit = 0;

static int func_quit(char **words, int word, int devno)
{
        UNUSED(words);
        UNUSED(word);
        UNUSED(devno);
        quit = 1;
        return 0;
}

struct lcmd lcmds[] = {
        {
                .name   = "help",
                .syntax = "",
                .desc   = "List commands",
                .func   = func_help,
        },
        {
                .name   = "link",
                .syntax = " [DEV]",
                .desc   = "Link and port set-up and state",
                .func   = func_link,
        },
        {
                .name   = "stats",
                .syntax = " [DEV]",
                .desc   = "Statistics",
                .func   = func_stats,
        },
        {
                .name   = "info",
                .syntax = " [DEV]",
                .desc   = "Get device information",
                .func   = func_info,
        },
        {
                .name   = "clkdiv",
                .syntax = " DEV RUN [START]",
                .desc   = "Set clock divisor",
                .func   = func_speed,
        },
        {
                .name   = "q",
                .syntax = "",
                .desc   = "Quit",
                .func   = func_quit,
        },
        {
                .name   = "x",
                .syntax = " DEV [PATH_0 .. PATH_N] NODE_ID",
                .desc   = "Transmit one packet",
                .func   = func_x,
        },
        {
                .name   = "r",
                .syntax = " [DEV]",
                .desc   = "Receive messages",
                .func   = func_r,
        },
        { .name = NULL },
};

static char *hints(const char *buf, int *color, int *bold) {
        for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                if (0 == strcmp(buf, cmd->name)) {
                        *color = 35;
                        *bold = 0;
                        return (char *) cmd->syntax;
                }
        }
        return NULL;
}


#include "linenoise.h"
void commandloop(void)
{
	int i;

        char *in_buf = NULL;
	int in_buf_pos = 0;
	int newtoken, word;
	char *words[32];
	int command;

        linenoiseSetHintsCallback(hints);
        linenoiseHistorySetMaxLen(12);
	printf("Starting Packet processing loop. Enter Command:\n\n");
	quit = 0;
	command = 1;
	while (quit == 0) {
		if (command == 1) {
			command = 0;
		}

                free(in_buf);
                in_buf = linenoise(PROMPT);
                if (NULL == in_buf) {
                        printf("ERROR: linenoise...\n");
                }
                in_buf_pos = strlen(in_buf);
                command = !!in_buf_pos;

		/* if one line of input completed then execute it */
		if (command == 0)
			continue;

                linenoiseHistoryAdd(in_buf); /* Add to the history. */

		/* Parse buffer into words */
		newtoken = 1;
		word = 0;
		for (i = 0; i < in_buf_pos; i++) {
			if ((in_buf[i] == ' ') || (in_buf[i] == '\t')) {
				in_buf[i] = '\0';
				newtoken = 1;
				continue;
			}

			/* process one word */
			if (newtoken) {
				words[word++] = &in_buf[i];
				newtoken = 0;
			}
		}
		in_buf_pos = 0;

		/* Parse words to a command */
		if (word < 1) {
			printf(" invalid command\n");
			continue;
		}

                /* maybe parse optional devno */
                int devno = DEVNO_NONE;
                if (1 < word ) {
                        char *endptr;
                        devno = strtoul(words[1], &endptr, 0);
                        if (endptr == words[1]) {
                                devno = DEVNO_BAD;
                        } else if (devno < 0 || devno >= nospw) {
                                devno = DEVNO_BAD;
                        }
                }

                int cmdfound = 0;
                for (const struct lcmd *cmd = &lcmds[0]; cmd->name; cmd++) {
                        if (0 == strcmp(cmd->name, words[0])) {
                                //printf("match: %s\n", cmd->name);
                                cmd->func(words, word, devno);
                                cmdfound = 1;
                                break;
                        }
                }
                if (!cmdfound) {
                        printf("%s: command not found. See 'help'.\n", words[0]);
                }

                fflush(NULL);
	}
}

int dma_process_rx(struct grspw_device *dev)
{
	int cnt;
	struct grspw_list lst;
	struct grspw_pkt *pkt;
	unsigned char *c;

	/* Prepare receiver with packet buffers */
	if (dev->rx_list_cnt > 0) {
		cnt = grspw_dma_rx_prepare(dev->dev.dma[0], &dev->rx_list);
		if (cnt < 0) {
			printf("rx_prep failed %d\n", cnt);
			return -1;
		}
if (1) {
		printf("GRSPW%d: Prepared %d RX packet buffers for future "
		       "reception\n", dev->dev.index, dev->rx_list_cnt);
}
		dev->rx_list_cnt -= cnt;
	}

	/* Try to receive packets on receiver interface */
	grspw_list_clr(&lst);
	cnt = grspw_dma_rx_recv(dev->dev.dma[0], &lst);
	if (cnt < 0) {
		printf("rx_recv failed %d\n", cnt);
		return -1;
	}
	if (cnt > 0) {
		printf("GRSPW%d: Recevied %d packets\n", dev->dev.index, cnt);
		for (pkt = lst.head; pkt; pkt = pkt->next) {
			if ((pkt->flags & RXPKT_FLAG_RX) == 0) {
				printf(" PKT not received.. buf ret\n");
				continue;
			} else if (pkt->flags &
			           (RXPKT_FLAG_EEOP | RXPKT_FLAG_TRUNK)) {
				printf(" PKT RX errors:");
				if (pkt->flags & RXPKT_FLAG_TRUNK)
					printf(" truncated");
				if (pkt->flags & RXPKT_FLAG_EEOP)
					printf(" EEP");
				printf(" (0x%x)", pkt->flags);
			} else
				printf(" PKT");
			c = (unsigned char *)pkt->data;
			printf(" of length %d bytes, 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x...\n",
				(int) pkt->dlen, c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
		}

		/* Reuse packet buffers by moving packets to rx_list */
		grspw_list_append_list(&dev->rx_list, &lst);
		dev->rx_list_cnt += cnt;
	}
	return 0;
}

int dma_process_tx(struct grspw_device *dev)
{
	int cnt;
	struct grspw_list lst;
	struct grspw_pkt *pkt;
	unsigned char *c;

	/*** TX PART ***/

	/* Reclaim already transmitted buffers */
	grspw_list_clr(&lst);
	cnt = grspw_dma_tx_reclaim(dev->dev.dma[0], &lst);
	if (cnt < 0 ) {
		printf("tx_reclaim failed %d\n", cnt);
		exit(0);
	}
	/* put sent packets in end of send queue for retransmission */
	if (cnt > 0) {
		/*printf("GRSPW%d: Reclaimed %d TX packet buffers\n",
			dev->index, cnt);*/
		/* Clear transmission flags */
		pkt = lst.head;
		while (pkt) {
			if ((pkt->flags & TXPKT_FLAG_TX) == 0)
				printf(" One Packet TX failed\n");
			else if (pkt->flags & TXPKT_FLAG_LINKERR)
				printf(" One Packet with TX errors\n");
			pkt = pkt->next;
		}

		grspw_list_append_list(&dev->tx_buf_list, &lst);
		dev->tx_buf_list_cnt += cnt;
	}

	/* Send packets in the tx_list queue */
	if (dev->tx_list_cnt > 0) {
			printf("GRSPW%d: Sending %d packets\n", dev->dev.index,
				dev->tx_list_cnt);
			for (pkt = dev->tx_list.head; pkt; pkt = pkt->next) {
				printf(" PKT of length %lu bytes,", pkt->hlen+pkt->dlen);
				for (size_t i = 0; i < pkt->hlen+pkt->dlen && i < 8; i++) {
					if (i < pkt->hlen) 
						c = i + (unsigned char *)pkt->hdr;
					else
						c = i - pkt->hlen + (unsigned char *)pkt->data;
					printf(" 0x%02x", *c);
				}
				printf("\n");
			}
			cnt = grspw_dma_tx_send(dev->dev.dma[0], &dev->tx_list);
			if (cnt < 0) {
				printf("tx_send failed %d\n", cnt);
				exit(0);
			}
			dev->tx_list_cnt -= cnt;
	}
	return 0;
}

