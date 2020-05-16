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

#ifndef DRV_REGS_GRSPW_H
#define DRV_REGS_GRSPW_H

#include <stdint.h>

struct grspw_dma_regs {
        uint32_t ctrl;          /* DMA Channel Control */
        uint32_t rxmax;         /* RX Max Packet Length */
        uint32_t txdesc;        /* TX Descriptor Base/Current */
        uint32_t rxdesc;        /* RX Descriptor Base/Current */
        uint32_t addr;          /* Address Register */
        uint32_t resv[3];
};

struct grspw_regs {
        uint32_t ctrl;
        uint32_t status;
        uint32_t nodeaddr;
        uint32_t clkdiv;
        uint32_t destkey;
        uint32_t time;
        uint32_t timer;         /* Used only in GRSPW1 */
        uint32_t resv1;

        /* DMA Registers, ctrl.NCH determines number of ports,
         * up to 4 channels are supported
         */
        struct grspw_dma_regs dma[4];
};

/* GRSPW - Control Register - 0x00 */
#define GRSPW_CTRL_RA_BIT       31
#define GRSPW_CTRL_RX_BIT       30
#define GRSPW_CTRL_RC_BIT       29
#define GRSPW_CTRL_NCH_BIT      27
#define GRSPW_CTRL_PO_BIT       26
#define GRSPW_CTRL_ID_BIT       24
#define GRSPW_CTRL_LE_BIT       22
#define GRSPW_CTRL_PS_BIT       21
#define GRSPW_CTRL_NP_BIT       20
#define GRSPW_CTRL_RD_BIT       17
#define GRSPW_CTRL_RE_BIT       16
#define GRSPW_CTRL_TF_BIT       12
#define GRSPW_CTRL_TR_BIT       11
#define GRSPW_CTRL_TT_BIT       10
#define GRSPW_CTRL_LI_BIT       9
#define GRSPW_CTRL_TQ_BIT       8
#define GRSPW_CTRL_RS_BIT       6
#define GRSPW_CTRL_PM_BIT       5
#define GRSPW_CTRL_TI_BIT       4
#define GRSPW_CTRL_IE_BIT       3
#define GRSPW_CTRL_AS_BIT       2
#define GRSPW_CTRL_LS_BIT       1
#define GRSPW_CTRL_LD_BIT       0

#define GRSPW_CTRL_RA           (1<<GRSPW_CTRL_RA_BIT)
#define GRSPW_CTRL_RX           (1<<GRSPW_CTRL_RX_BIT)
#define GRSPW_CTRL_RC           (1<<GRSPW_CTRL_RC_BIT)
#define GRSPW_CTRL_NCH          (0x3<<GRSPW_CTRL_NCH_BIT)
#define GRSPW_CTRL_PO           (1<<GRSPW_CTRL_PO_BIT)
#define GRSPW_CTRL_ID           (1<<GRSPW_CTRL_ID_BIT)
#define GRSPW_CTRL_LE           (1<<GRSPW_CTRL_LE_BIT)
#define GRSPW_CTRL_PS           (1<<GRSPW_CTRL_PS_BIT)
#define GRSPW_CTRL_NP           (1<<GRSPW_CTRL_NP_BIT)
#define GRSPW_CTRL_RD           (1<<GRSPW_CTRL_RD_BIT)
#define GRSPW_CTRL_RE           (1<<GRSPW_CTRL_RE_BIT)
#define GRSPW_CTRL_TF           (1<<GRSPW_CTRL_TF_BIT)
#define GRSPW_CTRL_TR           (1<<GRSPW_CTRL_TR_BIT)
#define GRSPW_CTRL_TT           (1<<GRSPW_CTRL_TT_BIT)
#define GRSPW_CTRL_LI           (1<<GRSPW_CTRL_LI_BIT)
#define GRSPW_CTRL_TQ           (1<<GRSPW_CTRL_TQ_BIT)
#define GRSPW_CTRL_RS           (1<<GRSPW_CTRL_RS_BIT)
#define GRSPW_CTRL_PM           (1<<GRSPW_CTRL_PM_BIT)
#define GRSPW_CTRL_TI           (1<<GRSPW_CTRL_TI_BIT)
#define GRSPW_CTRL_IE           (1<<GRSPW_CTRL_IE_BIT)
#define GRSPW_CTRL_AS           (1<<GRSPW_CTRL_AS_BIT)
#define GRSPW_CTRL_LS           (1<<GRSPW_CTRL_LS_BIT)
#define GRSPW_CTRL_LD           (1<<GRSPW_CTRL_LD_BIT)

/* GRSPW - Status Register - 0x04 */
#define GRSPW_STS_LS_BIT        21
#define GRSPW_STS_AP_BIT        9
#define GRSPW_STS_EE_BIT        8
#define GRSPW_STS_IA_BIT        7
#define GRSPW_STS_WE_BIT        6       /* GRSPW1 */
#define GRSPW_STS_PE_BIT        4
#define GRSPW_STS_DE_BIT        3
#define GRSPW_STS_ER_BIT        2
#define GRSPW_STS_CE_BIT        1
#define GRSPW_STS_TO_BIT        0

#define GRSPW_STS_LS            (0x7<<GRSPW_STS_LS_BIT)
#define GRSPW_STS_AP            (1<<GRSPW_STS_AP_BIT)
#define GRSPW_STS_EE            (1<<GRSPW_STS_EE_BIT)
#define GRSPW_STS_IA            (1<<GRSPW_STS_IA_BIT)
#define GRSPW_STS_WE            (1<<GRSPW_STS_WE_BIT)   /* GRSPW1 */
#define GRSPW_STS_PE            (1<<GRSPW_STS_PE_BIT)
#define GRSPW_STS_DE            (1<<GRSPW_STS_DE_BIT)
#define GRSPW_STS_ER            (1<<GRSPW_STS_ER_BIT)
#define GRSPW_STS_CE            (1<<GRSPW_STS_CE_BIT)
#define GRSPW_STS_TO            (1<<GRSPW_STS_TO_BIT)

/* GRSPW - Clock Divisor Register - 0x0C */
#define GRSPW_CLKDIV_START_BIT  8
#define GRSPW_CLKDIV_RUN_BIT    0
#define GRSPW_CLKDIV_START      (0xff<<GRSPW_CLKDIV_START_BIT)
#define GRSPW_CLKDIV_RUN        (0xff<<GRSPW_CLKDIV_RUN_BIT)
#define GRSPW_CLKDIV_MASK       (GRSPW_CLKDIV_START|GRSPW_CLKDIV_RUN)

/* GRSPW - Destination key Register - 0x10 */
#define GRSPW_DK_DESTKEY_BIT    0
#define GRSPW_DK_DESTKEY        (0xff<<GRSPW_DK_DESTKEY_BIT)

/* GRSPW - Time Register - 0x14 */
#define GRSPW_TIME_TCTRL_BIT    6
#define GRSPW_TIME_TCNT_BIT     0
#define GRSPW_TIME_CTRL         (0x3<<GRSPW_TIME_TCTRL_BIT)
#define GRSPW_TIME_TCNT         (0x3f<<GRSPW_TIME_TCNT_BIT)

/* GRSPW - DMA Control Register - 0x20*N */
#define GRSPW_DMACTRL_LE_BIT    16
#define GRSPW_DMACTRL_SP_BIT    15
#define GRSPW_DMACTRL_SA_BIT    14
#define GRSPW_DMACTRL_EN_BIT    13
#define GRSPW_DMACTRL_NS_BIT    12
#define GRSPW_DMACTRL_RD_BIT    11
#define GRSPW_DMACTRL_RX_BIT    10
#define GRSPW_DMACTRL_AT_BIT    9
#define GRSPW_DMACTRL_RA_BIT    8
#define GRSPW_DMACTRL_TA_BIT    7
#define GRSPW_DMACTRL_PR_BIT    6
#define GRSPW_DMACTRL_PS_BIT    5
#define GRSPW_DMACTRL_AI_BIT    4
#define GRSPW_DMACTRL_RI_BIT    3
#define GRSPW_DMACTRL_TI_BIT    2
#define GRSPW_DMACTRL_RE_BIT    1
#define GRSPW_DMACTRL_TE_BIT    0

#define GRSPW_DMACTRL_LE        (1<<GRSPW_DMACTRL_LE_BIT)
#define GRSPW_DMACTRL_SP        (1<<GRSPW_DMACTRL_SP_BIT)
#define GRSPW_DMACTRL_SA        (1<<GRSPW_DMACTRL_SA_BIT)
#define GRSPW_DMACTRL_EN        (1<<GRSPW_DMACTRL_EN_BIT)
#define GRSPW_DMACTRL_NS        (1<<GRSPW_DMACTRL_NS_BIT)
#define GRSPW_DMACTRL_RD        (1<<GRSPW_DMACTRL_RD_BIT)
#define GRSPW_DMACTRL_RX        (1<<GRSPW_DMACTRL_RX_BIT)
#define GRSPW_DMACTRL_AT        (1<<GRSPW_DMACTRL_AT_BIT)
#define GRSPW_DMACTRL_RA        (1<<GRSPW_DMACTRL_RA_BIT)
#define GRSPW_DMACTRL_TA        (1<<GRSPW_DMACTRL_TA_BIT)
#define GRSPW_DMACTRL_PR        (1<<GRSPW_DMACTRL_PR_BIT)
#define GRSPW_DMACTRL_PS        (1<<GRSPW_DMACTRL_PS_BIT)
#define GRSPW_DMACTRL_AI        (1<<GRSPW_DMACTRL_AI_BIT)
#define GRSPW_DMACTRL_RI        (1<<GRSPW_DMACTRL_RI_BIT)
#define GRSPW_DMACTRL_TI        (1<<GRSPW_DMACTRL_TI_BIT)
#define GRSPW_DMACTRL_RE        (1<<GRSPW_DMACTRL_RE_BIT)
#define GRSPW_DMACTRL_TE        (1<<GRSPW_DMACTRL_TE_BIT)

/* RX Buffer Descriptor */
struct grspw_rxbd {
        uint32_t ctrl;
        uint32_t addr;
};

/* TX Buffer Descriptor */
struct grspw_txbd {
        uint32_t ctrl;
        uint32_t haddr;
        uint32_t dlen;
        uint32_t daddr;
};

/* GRSPW - DMA RXBD Ctrl */
#define GRSPW_RXBD_LEN_BIT 0
#define GRSPW_RXBD_EN_BIT  25
#define GRSPW_RXBD_LEN  (0x1ffffff<<GRSPW_RXBD_LEN_BIT)
#define GRSPW_RXBD_EN   (1<<25)
#define GRSPW_RXBD_WR   (1<<26)
#define GRSPW_RXBD_IE   (1<<27)
#define GRSPW_RXBD_EP   (1<<28)
#define GRSPW_RXBD_HC   (1<<29)
#define GRSPW_RXBD_DC   (1<<30)
#define GRSPW_RXBD_TR   (1<<31)

/* GRSPW - DMA TXBD Ctrl */
#define GRSPW_TXBD_EN_BIT  12
#define GRSPW_TXBD_HLEN (0xff<<0)
#define GRSPW_TXBD_NCL  (0xf<<8)
#define GRSPW_TXBD_EN   (1<<12)
#define GRSPW_TXBD_WR   (1<<13)
#define GRSPW_TXBD_IE   (1<<14)
#define GRSPW_TXBD_LE   (1<<15)
#define GRSPW_TXBD_HC   (1<<16)
#define GRSPW_TXBD_DC   (1<<17)

#endif

