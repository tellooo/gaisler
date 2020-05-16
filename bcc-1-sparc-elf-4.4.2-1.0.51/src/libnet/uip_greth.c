/*
 *  driver for Gaisler Ethernet Controller
 *  Based on rtems driver and greth ecos driver
 *  groups@menta.fr
 */

#include <uip.h>
#include <uip_arp.h>
#include <asm-leon/leon.h>
#include <asm-leon/leoncompat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/param.h>

/* Set to perms of:
   0 disables all debug output
   1 for enabling DEBUG_FUNCTION output
   2 for read output
   4 for write output
   8 for read/write output
*/
#define DEBUG 0 //(-1 & ~64) //-1&(~64))
//-1& ~(16)& ~(32))
//#define DEBUG (8)
//#define DEBUG (-1 & ~(16))

#include <uip_greth.h>

#define ARP_PERIOD_US 500000

static struct greth_softc *greth;

static unsigned char rxbuf[GRETH_RXBD_NUM*GRETH_RX_BUF_SIZE];
static unsigned char txbuf[GRETH_TXBD_NUM*GRETH_TX_BUF_SIZE];

static char rxbufr[1024*2];
static char txbufr[1024*2];

static int read_mii(int addr, volatile greth_regs *regs)
{
  DEBUG_FUNCTION();

  while (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY) {}
  GRETH_REGSAVE(regs->mdio, (0 << 11) | ((addr&0x1F) << 6) | 2);
  while (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY) {}
  if (!(GRETH_REGLOAD(regs->mdio) & GRETH_MII_NVALID)) {
    return (GRETH_REGLOAD(regs->mdio)>>16)&0xFFFF;
  }
  else {
    return -1;
  }
}

static void write_mii(int addr, int data, volatile greth_regs *regs)
{
  DEBUG_FUNCTION();

  while (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY) {}
  GRETH_REGSAVE(regs->mdio, ((data&0xFFFF)<<16) | (0 << 11) | ((addr&0x1F) << 6) | 1);
  while (GRETH_REGLOAD(regs->mdio) & GRETH_MII_BUSY) {}
}

/* Initialize the ethernet hardware */
void libio_uip_greth_initialize_hardware () {

  int i, tmp;
  volatile greth_regs *regs = greth->regs;
  volatile greth_bd *tx_bd, *rx_bd;
  unsigned long txmem_addr = (unsigned long)&(txbuf);
  unsigned long rxmem_addr = (unsigned long)&(rxbuf);

  DEBUG_FUNCTION();

  /* Reset the controller.  */
  GRETH_REGSAVE(regs->control,GRETH_RESET);
  GRETH_REGSAVE(regs->control,0);
  while (GRETH_REGLOAD(regs->control) & GRETH_RESET) {}

  /* Configure PHY  */
  write_mii(0, 0x8000, regs); // reset phy
  while ( (tmp=read_mii(0, regs)) & 0x8000) {}

  if (tmp & 0x1000) { // augo neg
    while ( !(read_mii(1, regs) & 0x20 ) ) {}
  }
  tmp = read_mii(0, regs);
  printf("GRETH Ethermac MAC at [0x%x]. Running %d Mbps %s duplex\n", (unsigned int)(regs), ((tmp&0x2040) == 0x2000) ? 100:10, (tmp&0x0100) ? "full":"half");
  if (tmp & 0x0100) {
    // set full duplex
    GRETH_REGORIN(regs->control,GRETH_FD);
  }

  /* Initialize TXBD pointer
   */
  greth->tx_bd_base = (volatile greth_bd *)((((unsigned int)&txbufr) + (1024-1)) & ~(1024-1));
  GRETH_REGSAVE(regs->tx_desc_p,(unsigned32)greth->tx_bd_base);
  tx_bd = (volatile greth_bd *) greth->tx_bd_base;

  /* Initialize RXBD pointer
   */
  greth->rx_bd_base = (volatile greth_bd *)((((unsigned int)&rxbufr) + (1024-1)) & ~(1024-1));
  GRETH_REGSAVE(regs->rx_desc_p,(unsigned32)(greth->rx_bd_base));
  rx_bd = (volatile greth_bd *) greth->rx_bd_base;

  /* Initialize pointers.
   */
  greth->rx_cur = 0;
  greth->tx_next = 0;
  greth->tx_last = 0;
  greth->tx_full = 0;

  /* Initialize TXBDs.
   */
  for(i = 0; i < GRETH_TXBD_NUM; i++) {
    DB1_printf("tx[%d]:0x%x\n",i,txmem_addr);
    GRETH_REGSAVE(tx_bd[i].stat, 0 /*GRETH_BD_EN*/);
    GRETH_REGSAVE(tx_bd[i].addr, txmem_addr);
    txmem_addr += GRETH_TX_BUF_SIZE;
  }
  GRETH_REGORIN(tx_bd[GRETH_TXBD_NUM - 1].stat,GRETH_BD_WR);

  /* Initialize RXBDs.
   */
  for(i = 0; i < GRETH_RXBD_NUM; i++) {
    DB1_printf("rx[%d]:0x%x\n",i,rxmem_addr);
    GRETH_REGSAVE(rx_bd[i].stat, GRETH_BD_EN);
    GRETH_REGSAVE(rx_bd[i].addr,rxmem_addr);
    rxmem_addr += GRETH_RX_BUF_SIZE;
  }
  GRETH_REGORIN(rx_bd[GRETH_RXBD_NUM - 1].stat,GRETH_BD_WR);

  /* Set default ethernet station address.
   */
  GRETH_REGSAVE(regs->esa_msb, greth->ac_enaddr[0] << 8  | greth->ac_enaddr[1]);
  GRETH_REGSAVE(regs->esa_lsb, greth->ac_enaddr[2] << 24 | greth->ac_enaddr[3] << 16 | greth->ac_enaddr[4] << 8  | greth->ac_enaddr[5]);

  /* Clear all pending interrupts
   */
  GRETH_REGSAVE(regs->status,0);

  /* Enable receiver and transmiter
   */
  GRETH_REGORIN(regs->control, GRETH_RXEN | GRETH_TXEN);
}

static int arp_ticks = 0;
unsigned int libio_uip_greth_read(void) {
  unsigned int bad,stat,len;
  unsigned int *count = (unsigned int *) 0;
  unsigned int *reload = (unsigned int *) 0;
  unsigned int *ctrl = (unsigned int *) 0;
  unsigned char *addr;
  int val1, val2, i, cur;
  
  DB64_printf("libio_uip_greth_read\n",0);

  if (!Timer_getTimer1(&count,&reload,&ctrl)) 
    return 0;
  
  val1 = *count;

  while (arp_ticks > 0) {
    do {
      cur = greth->rx_cur;

      if (!((stat = GRETH_REGLOAD(greth->rx_bd_base[cur].stat)) & GRETH_BD_EN)) {

        bad = 0;
        len = ((stat & GRETH_BD_LEN)<(UIP_BUFSIZE)?(stat & GRETH_BD_LEN):(UIP_BUFSIZE));

        // Check status for errors.
        if (stat & GRETH_RXBD_ERR_FT) {
          greth->rx_length_errors++;
          bad = 1;
        }
        if (stat & (GRETH_RXBD_ERR_AE | GRETH_RXBD_ERR_OE)) {
          greth->rx_frame_errors++;
          bad = 1;
        }
        if (stat & GRETH_RXBD_ERR_CRC) { //bdp->stat
          greth->rx_crc_errors++;
          bad = 1;
        }

        GRETH_REGANDIN(greth->rx_bd_base[cur].stat , ~GRETH_RXBD_STATUS);
        GRETH_REGORIN(greth->rx_bd_base[cur].stat, GRETH_BD_EN | ((cur == GRETH_RXBD_NUM_MASK) ? GRETH_BD_WR : 0));
        GRETH_REGORIN(greth->regs->control,GRETH_RXEN);
        greth->rx_cur = (greth->rx_cur + 1) % GRETH_RXBD_NUM;

        if (!bad) {

          addr = (unsigned char *)greth->rx_bd_base[cur].addr;

          DB8_printf(">");
          DB2_printf("Rx packet:0x%x (%d)\n",addr,len);

          // pass on the packet in the receive buffer
          for (i = 0;i < len;i++) {
            uip_buf[i] = addr[i];
          }

          for (i = 0; i < len; i++) {
            DB2_printf("%02x ",uip_buf[i]);
            if (i % 16 == 15) {
              DB2_printf("\n");
            }
          }
          DB2_printf("\n");

          greth->rx_packets++;
          return len;
        }
      }
      val2 = val1 - *count;
    } while (!val2);
    arp_ticks -= (val2 >= 0) ? val2 : val2+(*reload)+1;
  }

  arp_ticks = ARP_PERIOD_US;
  return 0;
}

void libio_uip_greth_send(void) {

  int ret; unsigned char *addr;
  unsigned int len,stat,i,cur;
  DEBUG_FUNCTION();

  cur = greth->tx_next;

  DB4_printf("Tx wait %d 0x%x\n",cur, &greth->tx_bd_base[cur].stat);
  /* wait for previous to finish */
  while (GRETH_REGLOAD(greth->tx_bd_base[cur].stat) & GRETH_BD_EN) ;
  DB4_printf("Tx start\n",cur);

  /* don't send long packets */
  if ( uip_len <= GRETH_TX_BUF_SIZE ) {

    addr = (unsigned char *)greth->tx_bd_base[cur].addr;

    /* copy packet data */
    for(i = 0; i < 40 + UIP_LLH_LEN;i++) {
      addr[i] = uip_buf[i];
    }

    for(; i < uip_len;i++) {
      addr[i] = uip_appdata[i-(40 + UIP_LLH_LEN)];
    }

    DB8_printf("<");
    DB4_printf("Tx packet[%d]:0x%x (len:%d)\n",cur,addr,uip_len);

    for (i = 0; i < uip_len; i++) {
      DB4_printf("%02x ",addr[i]);
      if (i % 16 == 15) {
        DB4_printf("\n");
      }
    }
    DB4_printf("\n");
    for (i = 0; i < uip_len; i++) {
	DB4_printf("%c ",isprint(addr[i]) ? addr[i] : ' ');
	if (i % 16 == 15) {
	    DB4_printf("\n");
      }
    }
    DB4_printf("\n");

    /* Clear all of the status flags.  */
    stat = GRETH_REGANDIN(greth->tx_bd_base[cur].stat,~GRETH_TXBD_STATUS);

    /* write buffer descriptor length and status */
    len = uip_len;
    stat = (len & GRETH_BD_LEN) | GRETH_BD_EN | ((cur == GRETH_TXBD_NUM_MASK) ? GRETH_BD_WR : 0);
    GRETH_REGORIN(greth->tx_bd_base[cur].stat,stat);
    GRETH_REGORIN(greth->regs->control,GRETH_TXEN);

    /* wait for this one to finish */
    while (GRETH_REGLOAD(greth->tx_bd_base[cur]) & GRETH_BD_EN) ;
    DB4_printf("Tx status: 0x%x\n",greth->tx_bd_base[cur].stat);

    greth->tx_packets++;
    greth->tx_next = (greth->tx_next + 1) % GRETH_TXBD_NUM;
  }
}

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static u8_t arptimer = 0;
int libio_uip_greth_init(struct greth_softc *o,void (*func_ip) (void),void (*func_udp) (void)) {

  u8_t i, arptimer;
  u16_t hostaddr[2];
  struct uip_eth_addr arp;

  uip_appcall = func_ip;
  uip_udp_appcall = func_udp;

  greth = o;
  arp_ticks = ARP_PERIOD_US;

  DEBUG_FUNCTION();

  /* Initialize the device driver. */
  libio_uip_greth_initialize_hardware();

  /* Initialize the uIP TCP/IP stack. */
  uip_init();

  /* Initialize the ethermac address */
  arp.addr[0] = greth->ac_enaddr[0];
  arp.addr[1] = greth->ac_enaddr[1];
  arp.addr[2] = greth->ac_enaddr[2];
  arp.addr[3] = greth->ac_enaddr[3];
  arp.addr[4] = greth->ac_enaddr[4];
  arp.addr[5] = greth->ac_enaddr[5];
  uip_setethaddr(arp);

  /* Initialize the ip addresses */
  uip_ipaddr(hostaddr,greth->ipaddr[0],greth->ipaddr[1],greth->ipaddr[2],greth->ipaddr[3]);
  uip_sethostaddr(hostaddr);
  uip_ipaddr(hostaddr,greth->dripaddr[0],greth->dripaddr[1],greth->dripaddr[2],greth->dripaddr[3]);
  uip_setdraddr(hostaddr);
  uip_ipaddr(hostaddr,greth->maskaddr[0],greth->maskaddr[1],greth->maskaddr[2],greth->maskaddr[3]);
  uip_setnetmask(hostaddr);

  arptimer = 0;
}

int libio_uip_greth_tick() {
  DB64_printf("libio_uip_greth_tick\n",0);

  u8_t i;

    /* Let the tapdev network device driver read an entire IP packet
       into the uip_buf. If it must wait for more than 0.5 seconds, it
       will return with the return value 0. If so, we know that it is
       time to call upon the uip_periodic(). Otherwise, the tapdev has
       received an IP packet that is to be processed by uIP. */
    uip_len = libio_uip_greth_read();
    if(uip_len == 0) {
      for(i = 0; i < UIP_CONNS; i++) {
        uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  libio_uip_greth_send();
	}
      }

#if UIP_UDP
      for(i = 0; i < UIP_UDP_CONNS; i++) {
	uip_udp_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  libio_uip_greth_send();
	}
      }
#endif /* UIP_UDP */

      /* Call the ARP timer function every 10 seconds. */
      if(++arptimer == 20) {
	uip_arp_timer();
	arptimer = 0;
      }

    } else {
      if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	uip_arp_ipin();
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  libio_uip_greth_send();
	}
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  libio_uip_greth_send();
	}
      }
    }

  return 0;
}
