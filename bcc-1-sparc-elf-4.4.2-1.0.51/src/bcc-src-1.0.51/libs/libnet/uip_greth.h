/* Gaisler Research ethernet MAC driver */
/* adapted from linux driver by Laurent Rougé (MENTA) */

#ifndef _GR_ETH_
#define _GR_ETH_

typedef unsigned int unsigned32;

/* Configuration Information */

typedef struct {
  unsigned32              base_address;
  unsigned32              vector;
  unsigned32              txd_count;
  unsigned32              rxd_count;
} greth_configuration_t;

/* Ethernet buffer descriptor */
typedef struct _greth_bd {
  volatile unsigned32 stat;
  volatile unsigned32 *addr;           /* Buffer address */
} greth_bd;

/* Ethernet configuration registers */

typedef struct _greth_regs {
   volatile unsigned32 control;      /* Ctrl Register */
   volatile unsigned32 status;       /* Status Register */
   volatile unsigned32 esa_msb;      /* Bit 47-32 of MAC address */
   volatile unsigned32 esa_lsb;      /* Bit 31-0 of MAC address */
   volatile unsigned32 mdio;         /* MDIO control and status */
   volatile unsigned32 tx_desc_p;    /* Transmit descriptor pointer */
   volatile unsigned32 rx_desc_p;    /* Receive descriptor pointer */
} greth_regs;

/*
 * Per-device data
 */
struct greth_softc
{
  volatile greth_regs *regs;
  volatile greth_bd *tx_bd_base;		        /* Address of Tx BDs. */
  volatile greth_bd *rx_bd_base;		        /* Address of Rx BDs. */

  unsigned int tx_next;
  unsigned int tx_last;
  unsigned int tx_full;
  unsigned int rx_cur;

  /* configuration */
  unsigned char ac_enaddr[6]; // used
  unsigned char ipaddr[4];
  unsigned char dripaddr[4]; // used
  unsigned char maskaddr[4]; // used

  /*
   * Statistics
   */
  unsigned long rx_length_errors;
  unsigned long rx_frame_errors;
  unsigned long rx_crc_errors;
  unsigned long rx_packets;
  unsigned long tx_packets;
};

int libio_uip_greth_init(struct greth_softc *o,void (*func_ip) (void),void (*func_udp) (void));

#define GRETH_FD 0x10
#define GRETH_RESET 0x40
#define PHY_ADDR 0
#define GRETH_MII_BUSY 0x8
#define GRETH_MII_NVALID 0x10

#define GRETH_BD_EN 0x800
#define GRETH_BD_WR 0x1000
#define GRETH_BD_IE 0x2000
#define GRETH_BD_LEN 0x7FF

#define GRETH_INT_TX 0x8
#define GRETH_TXEN 0x1
#define GRETH_TXI 0x4
#define GRETH_TXBD_STATUS 0xFFFFC000
#define GRETH_TXBD_ERR_UE 0x4000
#define GRETH_TXBD_ERR_AL 0x8000
#define GRETH_TX_BUF_SIZE 2048
#define GRETH_TXBD_NUM 2
#define GRETH_TXBD_NUM_MASK GRETH_TXBD_NUM-1

#define GRETH_INT_RX 0x4
#define GRETH_RXEN 0x2
#define GRETH_RXI 0x8
#define GRETH_RXBD_STATUS 0xFFFFC000
#define GRETH_RXBD_ERR_AE 0x4000
#define GRETH_RXBD_ERR_FT 0x8000
#define GRETH_RXBD_ERR_CRC 0x10000
#define GRETH_RXBD_ERR_OE 0x20000
#define GRETH_RX_BUF_SIZE 2048
#define GRETH_RXBD_NUM 2
#define GRETH_RXBD_NUM_MASK GRETH_RXBD_NUM-1

#define ASI_LEON_NOCACHE 0x01
/* do a virtual address read without cache */
static __inline__ unsigned long leon_readnobuffer_reg(unsigned long paddr)
{
        unsigned long retval;
        __asm__ __volatile__("lda [%1] %2, %0\n\t":
                             "=r"(retval): "r"(paddr), "i"(ASI_LEON_NOCACHE));
        return retval;
}

#define LEON_BYPASSCACHE_LOAD_PA(x)     leon_readnobuffer_reg ((unsigned long)(x))
#define LEON_BYPASSCACHE_STORE_PA(x,v)     (*((unsigned long*)x) = (v))

#define GRETH_REGLOAD(a)    (LEON_BYPASSCACHE_LOAD_PA(&(a)))
#define GRETH_REGSAVE(a,v)  (LEON_BYPASSCACHE_STORE_PA(&(a),v))
#define GRETH_REGORIN(a,v)  (GRETH_REGSAVE(a,(GRETH_REGLOAD(a) | (v))))
#define GRETH_REGANDIN(a,v) (GRETH_REGSAVE(a,(GRETH_REGLOAD(a) & (v))))

#define DBC_PRINTF(lvl,format, args...) do { \
  if (lvl & DEBUG) { \
    char buf[1024]; \
    sprintf(buf,format,##args); \
    printf(buf); \
  } \
} while(0)

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

#endif /* _GR_ETH_ */
