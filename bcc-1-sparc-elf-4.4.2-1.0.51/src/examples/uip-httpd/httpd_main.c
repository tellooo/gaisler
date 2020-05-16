/*
 * Copyright (c) 2001-2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: main.c,v 1.10.2.1 2003/10/04 22:54:17 adam Exp $
 *
 */


#include <uip/uip.h>
#include <uip/uip_arp.h>
#include <uip/uip_open_eth.h>
#include <uip/uip_greth.h>
#include "httpd.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

struct open_eth_softc oc;
struct greth_softc greth;

void httpd_appcall(void);
void void_appcall(void) { };

/*-----------------------------------------------------------------------------------*/
int
main(void)
{
  u8_t i, arptimer;

  
#ifdef USE_OPENCORES
  /* init hardware */
  oc.regs = 0xb0000000;
  oc.ac_enaddr[0] = 0x00;
  oc.ac_enaddr[1] = 0xbd;
  oc.ac_enaddr[2] = 0x3b;
  oc.ac_enaddr[3] = 0x33;
  oc.ac_enaddr[4] = 0x05;
  oc.ac_enaddr[5] = 0x71;

  oc.ipaddr[0] = 192;
  oc.ipaddr[1] = 168;
  oc.ipaddr[2] = 0;
  oc.ipaddr[3] = 80;

  oc.dripaddr[0] = 192;
  oc.dripaddr[1] = 168;
  oc.dripaddr[2] = 0;
  oc.dripaddr[3] = 1;

  oc.maskaddr[0] = 255;
  oc.maskaddr[1] = 255;
  oc.maskaddr[2] = 255;
  oc.maskaddr[3] = 0;
  libio_uip_open_eth_init(&oc,httpd_appcall,void_appcall);
#else    
  /* init hardware */
  greth.regs = (greth_regs *)0x80000b00; // Address of the GRETH !!!
  greth.ac_enaddr[0] = 0x00;
  greth.ac_enaddr[1] = 0xbd;
  greth.ac_enaddr[2] = 0x3b;
  greth.ac_enaddr[3] = 0x33;
  greth.ac_enaddr[4] = 0x05;
  greth.ac_enaddr[5] = 0x71;

  greth.ipaddr[0] = 192;
  greth.ipaddr[1] = 168;
  greth.ipaddr[2] = 0;
  greth.ipaddr[3] = 80;

  greth.dripaddr[0] = 192;
  greth.dripaddr[1] = 168;
  greth.dripaddr[2] = 0;
  greth.dripaddr[3] = 1;

  greth.maskaddr[0] = 255;
  greth.maskaddr[1] = 255;
  greth.maskaddr[2] = 255;
  greth.maskaddr[3] = 0;
  libio_uip_greth_init(&greth,httpd_appcall,void_appcall);
#endif
  
  /* Initialize the HTTP server. */
  httpd_init();
  
  arptimer = 0;
  
  while(1) {
#ifdef USE_OPENCORES
    libio_uip_open_eth_tick();
#else
    libio_uip_greth_tick();
#endif    
  }
    
  return 0;
}
/*-----------------------------------------------------------------------------------*/
void
uip_log(char *m)
{
  printf("uIP log message: %s\n", m);
}
/*-----------------------------------------------------------------------------------*/
