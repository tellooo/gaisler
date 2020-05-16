/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>

#include "netif/etharp.h"
#include "ethernetif.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/* Forward declarations. */
static void  ethernetif_input(struct netif *netif);
static err_t ethernetif_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);

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


static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;

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
  
  
  /* set MAC hardware address length */
  netif->hwaddr_len = 6;
  
  /* set MAC hardware address */
  //todo: netif->hwaddr[0] = ;
  //todo:  ...
  //todo: netif->hwaddr[5] = ;

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* broadcast capability */
  netif->flags = NETIF_FLAG_BROADCAST;
 
  /* Do whatever else is needed to initialize interface. */  
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;

  //todo: initiate transfer();
  
#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
    //todo: send data from(q->payload, q->len);
  }

  //todo: signal that packet should be sent();

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif
  
#if LINK_STATS
  lwip_stats.link.xmit++;
#endif /* LINK_STATS */      

  return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */

static struct pbuf *
low_level_input(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  //todo: len = ;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE;						/* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable. */
      //todo: read data into(q->payload, q->len);
    }
    //todo: acknowledge that packet has been read();

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

#if LINK_STATS
    lwip_stats.link.recv++;
#endif /* LINK_STATS */      
  } else {
    //todo: drop packet();
#if LINK_STATS
    lwip_stats.link.memerr++;
    lwip_stats.link.drop++;
#endif /* LINK_STATS */      
  }

  return p;  
}

/*
 * ethernetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t
ethernetif_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
  
 /* resolve hardware address, then send (or queue) packet */
  return etharp_output(netif, ipaddr, p);
 
}

/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */

static void
ethernetif_input(struct netif *netif)
{
  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  ethernetif = netif->state;
  
  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* no packet could be read, silently ignore this */
  if (p == NULL) return;
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

#if LINK_STATS
  lwip_stats.link.recv++;
#endif /* LINK_STATS */

  ethhdr = p->payload;
    
  switch (htons(ethhdr->type)) {
  /* IP packet? */
  case ETHTYPE_IP:
    /* update ARP table */
    etharp_ip_input(netif, p);
    /* skip Ethernet header */
    pbuf_header(p, -sizeof(struct eth_hdr));
    /* pass to network layer */
    netif->input(p, netif);
    break;
      
    case ETHTYPE_ARP:
      /* pass p to ARP module  */
      etharp_arp_input(netif, ethernetif->ethaddr, p);
      break;
    default:
      pbuf_free(p);
      p = NULL;
      break;
  }
}

static void
arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/*
 * ethernetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  
  if (ethernetif == NULL)
  {
  	LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
  	return ERR_MEM;
  }
  
  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = ethernetif_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);

  etharp_init();

  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);

  return ERR_OK;
}

