/*
 *  driver for Opencores Ethernet Controller
 *  Based on rtems driver and open_eth ecos driver
 *
 */

#include <uip.h>
#include <uip_arp.h>
#include <asm-leon/leon.h>
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
#define DEBUG (0)
//#define DEBUG (8)
//#define DEBUG (-1 & ~(16))

#include <uip_open_eth.h> 

#define OETH_RXBD_NUM 2
#define OETH_TXBD_NUM 2
#define ARP_PERIOD_US 500000

static struct open_eth_softc *oc;
static unsigned char rxbuff[OETH_RXBD_NUM*OETH_RX_BUFF_SIZE];
static unsigned char txbuff[OETH_TXBD_NUM*OETH_TX_BUFF_SIZE];

static unsigned int read_mii(unsigned int addr) {
  
    DEBUG_FUNCTION();
    while (oc->regs->miistatus & OETH_MIISTATUS_BUSY) {}
    oc->regs->miiaddress = addr << 8;
    oc->regs->miicommand = OETH_MIICOMMAND_RSTAT;
    while (oc->regs->miistatus & OETH_MIISTATUS_BUSY) {}
    if (!(oc->regs->miistatus & OETH_MIISTATUS_NVALID))
        return(oc->regs->miirx_data);
    else {
	printf("open_eth: failed to read mii\n");
	return (0);
    }
}

static void write_mii(unsigned int addr, unsigned int data) {
  
    DEBUG_FUNCTION();
  
    while (oc->regs->miistatus & OETH_MIISTATUS_BUSY) {}
    oc->regs->miiaddress = addr << 8;
    oc->regs->miitx_data = data;
    oc->regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;
    while (oc->regs->miistatus & OETH_MIISTATUS_BUSY) {}
}

/* Initialize the ethernet hardware */
void libio_uip_open_eth_initialize_hardware () {
  
    int i;
    int mii_cr = 0;
    volatile oeth_regs *regs = oc->regs;
    unsigned long rxmem_addr = (unsigned long)&(rxbuff);
    unsigned long txmem_addr = (unsigned long)&(txbuff);

    DEBUG_FUNCTION(); 
  
    /* Reset the controller.  */
    regs->ctrlmoder = 0;
    regs->moder = OETH_MODER_RST;	/* Reset ON */
    regs->moder = 0;			/* Reset OFF */

    /* Initialize transmit pointers. */
    oc->rx_cur = 0;
    oc->tx_next = 0;
    oc->tx_last = 0;
    oc->tx_full = 0;
    
    /* reset PHY and wait for complettion */
    //write_mii(PHY_CNTL_REG, (oc->en100MHz ? PHY_CNTL_SPEED | PHY_CNTL_ANEG_EN | PHY_CNTL_DPLX | PHY_CNTL_ANEG_RST : 0) | PHY_CNTL_RST); 
    /* wait for reset */
    //while (read_mii(PHY_CNTL_REG) & PHY_CNTL_RST) {}
    //DB1_printf("open_eth: driver attached, PHY status : 0x%04x\n", read_mii(PHY_INT_REG));

    /* Set PHY to show Tx status, Rx status and Link status */
    regs->miiaddress = 20<<8;
    regs->miitx_data = 0x1422;
    regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;
    
    // switch to 10 mbit ethernet
    regs->miiaddress = 0;
    regs->miitx_data = 0;
    regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;

    
    /* Setting TXBD base to oc->txbufs  */
    regs->tx_bd_num = OETH_TXBD_NUM;
    oc->tx_bd = (oeth_bd *)OETH_BD_BASE(regs);
    oc->rx_bd = ((oeth_bd *)OETH_BD_BASE(regs)) + OETH_TXBD_NUM;
    
    regs->packet_len = 0x00400600;/* Set min/max packet length */
    regs->ipgt = 0x00000015; /* Set IPGT register to recomended value */
    regs->ipgr1 = 0x0000000c; /* Set IPGR1 register to recomended value */
    regs->ipgr2 = 0x00000012; /* Set IPGR2 register to recomended value */
    regs->collconf = 0x000f003f; /* Set COLLCONF register to recomended value */
    
    /* Initialize TXBDs. */
    for(i = 0; i < OETH_TXBD_NUM; i++) {
      DB1_printf("tx[%d]:0x%x\n",i,txmem_addr);
      OETH_REGSAVE(oc->tx_bd[i].len_status , OETH_TX_BD_PAD | OETH_TX_BD_CRC | OETH_TX_BD_IRQ); 
      OETH_REGSAVE(oc->tx_bd[i].addr , txmem_addr); 
      txmem_addr += OETH_TX_BUFF_SIZE;
    }
    OETH_REGORIN(oc->tx_bd[OETH_TXBD_NUM - 1].len_status , OETH_TX_BD_WRAP); 
    
    /* Initialize RXBDs. */
    for(i = 0; i < OETH_RXBD_NUM; i++) {
      DB1_printf("rx[%d]:0x%x\n",i,rxmem_addr);
      OETH_REGSAVE(oc->rx_bd[i].len_status , OETH_RX_BD_EMPTY | OETH_RX_BD_IRQ); 
      OETH_REGSAVE(oc->rx_bd[i].addr , rxmem_addr); 
      rxmem_addr += OETH_RX_BUFF_SIZE;
    }
    OETH_REGORIN(oc->rx_bd[OETH_RXBD_NUM - 1].len_status , OETH_RX_BD_WRAP); 
 
    /* set ethernet address.  */
    regs->mac_addr1 = oc->ac_enaddr[0] << 8  | oc->ac_enaddr[1];
    regs->mac_addr0 = oc->ac_enaddr[2] << 24 | oc->ac_enaddr[3] << 16 |
                      oc->ac_enaddr[4] << 8  | oc->ac_enaddr[5];
    
    /* clear all pending interrupts */
    regs->int_src = 0xffffffff;
    /* Enable interrupt sources. */
    regs->int_mask = 0;

    /* MAC mode register: PAD, IFG, CRCEN */
    regs->moder = OETH_MODER_PAD | OETH_MODER_CRCEN ;//| ((read_mii(PHY_CNTL_REG) & PHY_CNTL_DPLX) << 2);
    regs->moder |= OETH_MODER_RXEN | OETH_MODER_TXEN;
}

void libio_uip_open_eth_stop ()
{
  DEBUG_FUNCTION();
  
  oc->regs->moder = 0;		/* RX/TX OFF */
  oc->regs->moder = OETH_MODER_RST;	/* Reset ON */
  oc->regs->moder = 0;		/* Reset OFF */
}

static int arp_ticks = 0;
unsigned int libio_uip_open_eth_read(void) {
  
  unsigned int ret,bad,len_status,len;
  unsigned int *count,*reload,*ctrl;
  unsigned char *addr;
  int val1, val2, i, cur;
  
  if (!Timer_getTimer1(&count,&reload,&ctrl)) 
    return 0;

  val1 = *count;

  while (arp_ticks > 0) {
    do {
      cur = oc->rx_cur;
      if (!((len_status = oc->rx_bd[cur].len_status) & OETH_RX_BD_EMPTY)) {

        bad = 0;
        len = min(len_status >> 16,UIP_BUFSIZE);

        OPENETH_UPDATE_STATUS(oc,len_status,bad);
        OETH_REGANDIN(oc->rx_bd[cur].len_status , ~OETH_RX_BD_STATS); 
        OETH_REGORIN(oc->rx_bd[cur].len_status , OETH_RX_BD_EMPTY); 
        oc->rx_cur = (oc->rx_cur + 1) % OETH_RXBD_NUM;
        
        if (!bad) {

          addr = (unsigned char *)oc->rx_bd[cur].addr;
          
          DB8_printf(">");
          DB2_printf("Rx packet:0x%x (%d)\n",addr,len);


          /* pass on the packet in the receive buffer */
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
          
          oc->rxPackets++;
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

void libio_uip_open_eth_send(void) {
  
  int ret; unsigned char *addr;
  unsigned int len,len_status,i,cur;
  DEBUG_FUNCTION();

  cur = oc->tx_next;
  
  /* wait for previous to finish */
  while (oc->tx_bd[cur].len_status & OETH_TX_BD_READY) ;

  /* don't send long packets */
  if ( uip_len <= OETH_TX_BUFF_SIZE ) {
    
    addr = (unsigned char *)oc->tx_bd[cur].addr;

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

    /* Clear all of the status flags.  */
    len_status = oc->tx_bd[cur].len_status & ~OETH_TX_BD_STATS;
  
    /* If the frame is short, tell CPM to pad it.  */
    len = uip_len;
    if (len < ET_MINLEN) {
      len_status |= OETH_TX_BD_PAD;
      len = ET_MINLEN;
    }
    else
      len_status &= ~OETH_TX_BD_PAD;
    
    /* write buffer descriptor length and status */
    len_status &= 0x0000ffff;
    len_status |= (len << 16) | (OETH_TX_BD_READY | OETH_TX_BD_CRC);
    oc->tx_bd[cur].len_status = len_status;

    /* wait for previous to finish */
    while (oc->tx_bd[cur].len_status & OETH_TX_BD_READY) ;
    DB64_printf("Tx status: 0x%x\n",oc->tx_bd[cur].len_status);
    
    
    oc->tx_next = (oc->tx_next + 1) % OETH_TXBD_NUM;
  }
}  

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

static u8_t arptimer = 0;
int libio_uip_open_eth_init(struct open_eth_softc *o,void (*func_ip) (void),void (*func_udp) (void)) {
  
  u8_t i, arptimer;
  u16_t hostaddr[2];
  struct uip_eth_addr arp;

  uip_appcall = func_ip;
  uip_udp_appcall = func_udp;
  
  oc = o;
  arp_ticks = ARP_PERIOD_US;
  
  DEBUG_FUNCTION();

  /* Initialize the device driver. */ 
  libio_uip_open_eth_initialize_hardware();

  /* Initialize the uIP TCP/IP stack. */
  uip_init();

  /* Initialize the ethermac address */
  arp.addr[0] = oc->ac_enaddr[0];
  arp.addr[1] = oc->ac_enaddr[1];
  arp.addr[2] = oc->ac_enaddr[2];
  arp.addr[3] = oc->ac_enaddr[3];
  arp.addr[4] = oc->ac_enaddr[4];
  arp.addr[5] = oc->ac_enaddr[5];
  uip_setethaddr(arp);

  /* Initialize the ip addresses */
  uip_ipaddr(hostaddr,oc->ipaddr[0],oc->ipaddr[1],oc->ipaddr[2],oc->ipaddr[3]);
  uip_sethostaddr(hostaddr);
  uip_ipaddr(hostaddr,oc->dripaddr[0],oc->dripaddr[1],oc->dripaddr[2],oc->dripaddr[3]);
  uip_setdraddr(hostaddr);
  uip_ipaddr(hostaddr,oc->maskaddr[0],oc->maskaddr[1],oc->maskaddr[2],oc->maskaddr[3]);
  uip_setnetmask(hostaddr);
  
  arptimer = 0;
}

int libio_uip_open_eth_tick() {
  
  u8_t i;

    /* Let the tapdev network device driver read an entire IP packet
       into the uip_buf. If it must wait for more than 0.5 seconds, it
       will return with the return value 0. If so, we know that it is
       time to call upon the uip_periodic(). Otherwise, the tapdev has
       received an IP packet that is to be processed by uIP. */
    uip_len = libio_uip_open_eth_read();
    if(uip_len == 0) {
      for(i = 0; i < UIP_CONNS; i++) {
        uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  libio_uip_open_eth_send();
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
	  libio_uip_open_eth_send();
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
	  libio_uip_open_eth_send();
	}
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */	
	if(uip_len > 0) {	
	  libio_uip_open_eth_send();
	}
      }
    }
    
  return 0;
}
