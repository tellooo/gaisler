/* Opencores ethernet MAC driver */
/* adapted from linux driver by Jiri Gaisler */

#ifndef _OPEN_ETH_H_
#define _OPEN_ETH_H_

typedef unsigned int unsigned32;

/* Configuration Information */

typedef struct {
  unsigned32              base_address;
  unsigned32              vector;
  unsigned32              txd_count;
  unsigned32              rxd_count;
  unsigned32              en100MHz;
} open_eth_configuration_t;

/* Ethernet buffer descriptor */
typedef struct _oeth_bd {
    volatile unsigned32 len_status; /* Length and status */
    volatile unsigned32 *addr;      /* Buffer pointer */
} oeth_bd;

/* Ethernet configuration registers */

typedef struct _oeth_regs {
    volatile unsigned32 moder;       /* Mode Register */
    volatile unsigned32 int_src;     /* Interrupt Source Register */
    volatile unsigned32 int_mask;    /* Interrupt Mask Register */
    volatile unsigned32 ipgt;        /* Back to Bak Inter Packet Gap Register */
    volatile unsigned32 ipgr1;       /* Non Back to Back Inter Packet Gap Register 1 */
    volatile unsigned32 ipgr2;       /* Non Back to Back Inter Packet Gap Register 2 */
    volatile unsigned32 packet_len;  /* Packet Length Register (min. and max.) */
    volatile unsigned32 collconf;    /* Collision and Retry Configuration Register */
    volatile unsigned32 tx_bd_num;   /* Transmit Buffer Descriptor Number Register */
    volatile unsigned32 ctrlmoder;   /* Control Module Mode Register */
    volatile unsigned32 miimoder;    /* MII Mode Register */
    volatile unsigned32 miicommand;  /* MII Command Register */
    volatile unsigned32 miiaddress;  /* MII Address Register */
    volatile unsigned32 miitx_data;  /* MII Transmit Data Register */
    volatile unsigned32 miirx_data;  /* MII Receive Data Register */
    volatile unsigned32 miistatus;   /* MII Status Register */
    volatile unsigned32 mac_addr0;   /* MAC Individual Address Register 0 */
    volatile unsigned32 mac_addr1;   /* MAC Individual Address Register 1 */
    volatile unsigned32 hash_addr0;  /* Hash Register 0 */
    volatile unsigned32 hash_addr1;  /* Hash Register 1 */
    volatile unsigned32 txctrl;      /* Transmitter control register */
    unsigned32 empty[235];	     /* Unused space */
    oeth_bd xd[128];	             /* TX & RX descriptors */
} oeth_regs;

/*
 * Per-device data
 */
struct open_eth_softc {

    volatile oeth_regs *regs;
    volatile oeth_bd *rx_bd;		        /* Address of Rx BDs. */
    volatile oeth_bd *tx_bd;		        /* Address of Tx BDs. */

    unsigned int tx_next;			/* Next buffer to be sent */
    unsigned int tx_last;			/* Next buffer to be checked if packet sent */
    unsigned int tx_full;			/* Buffer ring fuul indicator */
    unsigned int rx_cur;                        /* Next buffer to be checked if packet received */
 
    /* configuration */
    unsigned int en100MHz;
    unsigned char ac_enaddr[6];
    unsigned int vector;
    unsigned char ipaddr[4];
    unsigned char dripaddr[4];
    unsigned char maskaddr[4];
  
    int acceptBroadcast;

    /*
     * Statistics
     */
    unsigned long rxInterrupts;
    unsigned long rxPackets;
    unsigned long rxLengthError;
    unsigned long rxNonOctet;
    unsigned long rxBadCRC;
    unsigned long rxOverrun;
    unsigned long rxMiss;
    unsigned long rxCollision;

    unsigned long txInterrupts;
    unsigned long txDeferred;
    unsigned long txHeartbeat;
    unsigned long txLateCollision;
    unsigned long txRetryLimit;
    unsigned long txUnderrun;
    unsigned long txLostCarrier;
    unsigned long txRawWait;
};
int libio_uip_open_eth_init(struct open_eth_softc *o,void (*func_ip) (void),void (*func_udp) (void));

#define OETH_BD_BASE(b)         (((unsigned long)(b)) + 0x400)

#define OETH_TOTAL_BD           128
#define OETH_MAXBUF_LEN         0x610
                                
/* Tx BD */                     
#define OETH_TX_BD_READY        0x8000 /* Tx BD Ready */
#define OETH_TX_BD_IRQ          0x4000 /* Tx BD IRQ Enable */
#define OETH_TX_BD_WRAP         0x2000 /* Tx BD Wrap (last BD) */
#define OETH_TX_BD_PAD          0x1000 /* Tx BD Pad Enable */
#define OETH_TX_BD_CRC          0x0800 /* Tx BD CRC Enable */
                                
#define OETH_TX_BD_UNDERRUN     0x0100 /* Tx BD Underrun Status */
#define OETH_TX_BD_RETRY        0x00F0 /* Tx BD Retry Status */
#define OETH_TX_BD_RETLIM       0x0008 /* Tx BD Retransmission Limit Status */
#define OETH_TX_BD_LATECOL      0x0004 /* Tx BD Late Collision Status */
#define OETH_TX_BD_DEFER        0x0002 /* Tx BD Defer Status */
#define OETH_TX_BD_CARRIER      0x0001 /* Tx BD Carrier Sense Lost Status */
#define OETH_TX_BD_STATS        (OETH_TX_BD_UNDERRUN            | \
                                OETH_TX_BD_RETRY                | \
                                OETH_TX_BD_RETLIM               | \
                                OETH_TX_BD_LATECOL              | \
                                OETH_TX_BD_DEFER                | \
                                OETH_TX_BD_CARRIER)
                                
/* Rx BD */                     
#define OETH_RX_BD_EMPTY        0x8000 /* Rx BD Empty */
#define OETH_RX_BD_IRQ          0x4000 /* Rx BD IRQ Enable */
#define OETH_RX_BD_WRAP         0x2000 /* Rx BD Wrap (last BD) */
                                
#define OETH_RX_BD_MISS         0x0080 /* Rx BD Miss Status */
#define OETH_RX_BD_OVERRUN      0x0040 /* Rx BD Overrun Status */
#define OETH_RX_BD_INVSIMB      0x0020 /* Rx BD Invalid Symbol Status */
#define OETH_RX_BD_DRIBBLE      0x0010 /* Rx BD Dribble Nibble Status */
#define OETH_RX_BD_TOOLONG      0x0008 /* Rx BD Too Long Status */
#define OETH_RX_BD_SHORT        0x0004 /* Rx BD Too Short Frame Status */
#define OETH_RX_BD_CRCERR       0x0002 /* Rx BD CRC Error Status */
#define OETH_RX_BD_LATECOL      0x0001 /* Rx BD Late Collision Status */
#define OETH_RX_BD_STATS        (OETH_RX_BD_MISS                | \
                                OETH_RX_BD_OVERRUN              | \
                                OETH_RX_BD_INVSIMB              | \
                                OETH_RX_BD_DRIBBLE              | \
                                OETH_RX_BD_TOOLONG              | \
                                OETH_RX_BD_SHORT                | \
                                OETH_RX_BD_CRCERR               | \
                                OETH_RX_BD_LATECOL)

/* MODER Register */
#define OETH_MODER_RXEN         0x00000001 /* Receive Enable  */
#define OETH_MODER_TXEN         0x00000002 /* Transmit Enable */
#define OETH_MODER_NOPRE        0x00000004 /* No Preamble  */
#define OETH_MODER_BRO          0x00000008 /* Reject Broadcast */
#define OETH_MODER_IAM          0x00000010 /* Use Individual Hash */
#define OETH_MODER_PRO          0x00000020 /* Promiscuous (receive all) */
#define OETH_MODER_IFG          0x00000040 /* Min. IFG not required */
#define OETH_MODER_LOOPBCK      0x00000080 /* Loop Back */
#define OETH_MODER_NOBCKOF      0x00000100 /* No Backoff */
#define OETH_MODER_EXDFREN      0x00000200 /* Excess Defer */
#define OETH_MODER_FULLD        0x00000400 /* Full Duplex */
#define OETH_MODER_RST          0x00000800 /* Reset MAC */
#define OETH_MODER_DLYCRCEN     0x00001000 /* Delayed CRC Enable */
#define OETH_MODER_CRCEN        0x00002000 /* CRC Enable */
#define OETH_MODER_HUGEN        0x00004000 /* Huge Enable */
#define OETH_MODER_PAD          0x00008000 /* Pad Enable */
#define OETH_MODER_RECSMALL     0x00010000 /* Receive Small */
 
/* Interrupt Source Register */
#define OETH_INT_TXB            0x00000001 /* Transmit Buffer IRQ */
#define OETH_INT_TXE            0x00000002 /* Transmit Error IRQ */
#define OETH_INT_RXF            0x00000004 /* Receive Frame IRQ */
#define OETH_INT_RXE            0x00000008 /* Receive Error IRQ */
#define OETH_INT_BUSY           0x00000010 /* Busy IRQ */
#define OETH_INT_TXC            0x00000020 /* Transmit Control Frame IRQ */
#define OETH_INT_RXC            0x00000040 /* Received Control Frame IRQ */

/* Interrupt Mask Register */
#define OETH_INT_MASK_TXB       0x00000001 /* Transmit Buffer IRQ Mask */
#define OETH_INT_MASK_TXE       0x00000002 /* Transmit Error IRQ Mask */
#define OETH_INT_MASK_RXF       0x00000004 /* Receive Frame IRQ Mask */
#define OETH_INT_MASK_RXE       0x00000008 /* Receive Error IRQ Mask */
#define OETH_INT_MASK_BUSY      0x00000010 /* Busy IRQ Mask */
#define OETH_INT_MASK_TXC       0x00000020 /* Transmit Control Frame IRQ Mask */
#define OETH_INT_MASK_RXC       0x00000040 /* Received Control Frame IRQ Mask */
 
/* Control Module Mode Register */
#define OETH_CTRLMODER_PASSALL  0x00000001 /* Pass Control Frames */
#define OETH_CTRLMODER_RXFLOW   0x00000002 /* Receive Control Flow Enable */
#define OETH_CTRLMODER_TXFLOW   0x00000004 /* Transmit Control Flow Enable */
                               
/* MII Mode Register */        
#define OETH_MIIMODER_CLKDIV    0x000000FF /* Clock Divider */
#define OETH_MIIMODER_NOPRE     0x00000100 /* No Preamble */
#define OETH_MIIMODER_RST       0x00000200 /* MIIM Reset */
 
/* MII Command Register */
#define OETH_MIICOMMAND_SCANSTAT  0x00000001 /* Scan Status */
#define OETH_MIICOMMAND_RSTAT     0x00000002 /* Read Status */
#define OETH_MIICOMMAND_WCTRLDATA 0x00000004 /* Write Control Data */
 
/* MII Address Register */
#define OETH_MIIADDRESS_FIAD    0x0000001F /* PHY Address */
#define OETH_MIIADDRESS_RGAD    0x00001F00 /* RGAD Address */
 
/* MII Status Register */
#define OETH_MIISTATUS_LINKFAIL 0x00000001 /* Link Fail */
#define OETH_MIISTATUS_BUSY     0x00000002 /* MII Busy */
#define OETH_MIISTATUS_NVALID   0x00000004 /* Data in MII Status Register is invalid */

// PHY Control Register
#define PHY_CNTL_REG		0x00
#define PHY_CNTL_RST		0x8000	// 1=PHY Reset
#define PHY_CNTL_LPBK		0x4000	// 1=PHY Loopback
#define PHY_CNTL_SPEED		0x2000	// 1=100Mbps, 0=10Mpbs
#define PHY_CNTL_ANEG_EN	0x1000 // 1=Enable Auto negotiation
#define PHY_CNTL_PDN		0x0800	// 1=PHY Power Down mode
#define PHY_CNTL_MII_DIS	0x0400	// 1=MII 4 bit interface disabled
#define PHY_CNTL_ANEG_RST	0x0200 // 1=Reset Auto negotiate
#define PHY_CNTL_DPLX		0x0100	// 1=Full Duplex, 0=Half Duplex
#define PHY_CNTL_COLTST		0x0080	// 1= MII Colision Test

// PHY Status Output (and Interrupt status) Register
#define PHY_INT_REG		0x12	// Status Output (Interrupt Status)
#define PHY_INT_INT		0x8000	// 1=bits have changed since last read
#define PHY_INT_LNKFAIL		0x4000	// 1=Link Not detected
#define PHY_INT_LOSSSYNC	0x2000	// 1=Descrambler has lost sync
#define PHY_INT_CWRD		0x1000	// 1=Invalid 4B5B code detected on rx
#define PHY_INT_SSD		0x0800	// 1=No Start Of Stream detected on rx
#define PHY_INT_ESD		0x0400	// 1=No End Of Stream detected on rx
#define PHY_INT_RPOL		0x0200	// 1=Reverse Polarity detected
#define PHY_INT_JAB		0x0100	// 1=Jabber detected
#define PHY_INT_SPDDET		0x0080	// 1=100Base-TX mode, 0=10Base-T mode
#define PHY_INT_DPLXDET		0x0040	// 1=Device in Full Duplex

// PHY Configuration Register 1
#define PHY_CFG1_REG		0x10
#define PHY_CFG1_LNKDIS		0x8000	// 1=Rx Link Detect Function disabled
#define PHY_CFG1_XMTDIS		0x4000	// 1=TP Transmitter Disabled
#define PHY_CFG1_XMTPDN		0x2000	// 1=TP Transmitter Powered Down
#define PHY_CFG1_BYPSCR		0x0400	// 1=Bypass scrambler/descrambler
#define PHY_CFG1_UNSCDS		0x0200	// 1=Unscramble Idle Reception Disable
#define PHY_CFG1_EQLZR		0x0100	// 1=Rx Equalizer Disabled
#define PHY_CFG1_CABLE		0x0080	// 1=STP(150ohm), 0=UTP(100ohm)
#define PHY_CFG1_RLVL0		0x0040	// 1=Rx Squelch level reduced by 4.5db
#define PHY_CFG1_TLVL_SHIFT	2	// Transmit Output Level Adjust
#define PHY_CFG1_TLVL_MASK	0x003C
#define PHY_CFG1_TRF_MASK	0x0003	// Transmitter Rise/Fall time

#define OETH_REGLOAD(a,v) (v = *(unsigned int *)(&(a)))
#define OETH_REGSAVE(a,v) (*(unsigned int *)(&(a)) = v)
#define OETH_REGORIN(a,v)  \
    { unsigned32 va;       \
      OETH_REGLOAD(a,va);  \
      va |= v;             \
      OETH_REGSAVE(a,va);  \
    }              
#define OETH_REGANDIN(a,v) \
    { unsigned32 va;       \
      OETH_REGLOAD(a,va);  \
      va &= v;             \
      OETH_REGSAVE(a,va);  \
    }              


#define OPENETH_UPDATE_STATUS(dp,len_status,bad) \
    if (len_status & (OETH_RX_BD_TOOLONG | OETH_RX_BD_SHORT)) { \
      dp->rxLengthError++; \
      bad = 1; \
    } \
    if (len_status & OETH_RX_BD_DRIBBLE) { \
      dp->rxNonOctet++; \
      bad = 1; \
    } \
    if (len_status & OETH_RX_BD_CRCERR) { \
      dp->rxBadCRC++; \
      bad = 1; \
    } \
    if (len_status & OETH_RX_BD_OVERRUN) { \
      dp->rxOverrun++; \
      bad = 1; \
    } \
    if (len_status & OETH_RX_BD_MISS){ \
      dp->rxMiss++; \
      bad = 1; \
    } \
    if (len_status & OETH_RX_BD_LATECOL){ \
      dp->rxCollision++; \
      bad = 1; \
    } 

#define	ET_MINLEN 64		/* minimum message length */

/* Buffer size  (if not XXBUF_PREALLOC */
#define OETH_MAX_FRAME_SIZE	((1518 + 7) & ~7)

/* Buffer size  */
#define OETH_RX_BUFF_SIZE	OETH_MAX_FRAME_SIZE
#define OETH_TX_BUFF_SIZE	OETH_MAX_FRAME_SIZE

//#ifdef DEBUG
#define DBC_PRINTF(lvl,format, args...) do { \
  if (lvl & DEBUG) { \
    char buf[1024]; \
    sprintf(buf,format,##args); \
    printf(buf); \
  } \
} while(0)
//#else
//#define DBC_PRINTF(lvl,format, args...)
//#endif

#define DB64_printf(format, args...) DBC_PRINTF(64,format,##args);
#define DB32_printf(format, args...) DBC_PRINTF(32,format,##args);
#define DB16_printf(format, args...) DBC_PRINTF(16,format,##args);
#define DB8_printf(format, args...) DBC_PRINTF(8,format,##args);
#define DB4_printf(format, args...) DBC_PRINTF(4,format,##args);
#define DB2_printf(format, args...) DBC_PRINTF(2,format,##args);
#define DB1_printf(format, args...) DBC_PRINTF(1,format,##args);
#define DB_printf(format, args...) DBC_PRINTF(0xffff,format,##args);

#if DEBUG & 1
#define DEBUG_FUNCTION() do { DB1_printf("# %s\n", __FUNCTION__); } while (0)
#else
#define DEBUG_FUNCTION() do {} while(0)
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#endif /* _OPEN_ETH_ */

