/************************************************************************/
/*   This file is a part of the mkprom3 boot-prom utility               */
/*   Copyright (C) 2004 Gaisler Research                                */
/*                                                                      */
/*   This library is free software; you can redistribute it and/or      */
/*   modify it under the terms of the GNU General Public                */
/*   License as published by the Free Software Foundation; either       */
/*   version 2 of the License, or (at your option) any later version.   */
/*                                                                      */
/*   See the file COPYING.GPL for the full details of the license.      */
/************************************************************************/

#define	USR	4/4
#define TXA	0/4

#define	SECNAME	16

typedef struct sectype
{
    unsigned int paddr;
    unsigned int raddr;
    unsigned int len;
    unsigned int comp;
    unsigned char name[SECNAME];
} tt;


extern char filename[];
extern struct sectype sections[];
extern int prot;
extern volatile unsigned int *_uaddr;

asm(
      ".text           \n"
      ".align 4        \n"
"getasr17:             \n"
      "retl            \n"
      "rd  %asr17, %o0 \n"
      );


void
putsx (s)
     unsigned char *s;
{
    volatile unsigned int *ubase;
    if (((getpsr()>>24) & 0x0f) != 3) ubase = (int *) 0x80000070;
    else ubase = _uaddr;
    while (s[0] != 0)
      {
	  while ((ubase[USR] & 4) == 0);
	  ubase[TXA] = *s;
	  s++;
      }
}

void
puthex (h)
     unsigned int h;
{
  int i = 0;
  char b[9];
  for (i = 0;i < 8;i++,h<<=4) {
    char c = ((h & 0xf0000000) >> 28) & 0xf;
    if (c >= 10) {
      c += 'a' - 10;
    } else {
      c += '0';
    }
    b[i] = c;
  }
  b[8] = 0;
  putsx(b);
}


__main ()
{
}

extern int ramsize, etext, freq, bmsg, _stack;
void clean ();
extern int Decode ();
extern char configmsg[];
extern int noinit;
extern int _entry[16];

main ()
{
    unsigned char *p;
    int paddr, raddr, len, secnum;
    void (*prog) ();
    /*char pbuf[8192];*/
    int cpundx, i;

    cpundx = (getasr17() >> 28) & 0x0f;
    prog = (void *) (_entry[cpundx]);
    if (cpundx == 0)
    {
      if (bmsg)
      {
	  putsx ("\n\n\r  MkProm LEON boot loader v2.0\n\r");
	  putsx ("  Copyright Gaisler Research - all right reserved\n\n\r");
	  putsx (configmsg);
	  putsx ("\n\r");
      }
      secnum = 0;

      while (sections[secnum].paddr)
      {
	  paddr = sections[secnum].paddr;
	  raddr = sections[secnum].raddr;
	  len = sections[secnum].len;
	  if (sections[secnum].comp)
	    {
		if (bmsg)
		  {
		      putsx ("  decompressing ");
		      putsx (sections[secnum].name);
		      putsx (" to 0x");
		      puthex(paddr);
		      putsx ("\n\r");
		  }
		if (Decode (raddr, paddr))
		  {
		      putsx ("  decompression failed \n\r");
		  }

	    }
	  else
	    {
		if (bmsg)
		  {
		      putsx ("  loading ");
		      putsx (sections[secnum].name);
		      putsx ("\n\r");
		  }
		mmov (raddr, paddr, len);
	    }
	  secnum++;
      }
      

      *((int *) (sections[0].paddr + 0x7e0)) = freq;

    
      
      /*
      if (bmsg)
      {
	putsx ("\n\r  starting ");
	putsx (filename);
	putsx ("\n\n\r");
      }
      */

    }
    /* reset cwp to 0 */
  __asm__ __volatile__(                                                   \
"        mov     %0,%%g1                                             \n\t"\
"        mov     %%sp,%%g2                                           \n\t"\
"        mov     %%fp,%%g3                                           \n\t"\
"        mov     %%psr,%%g4                                          \n\t"\
"        andn    %%g4,0x1f,%%g4                                      \n\t"\
"        wr      %%g4, 0x00, %%psr                                   \n\t"\
"        nop;nop;nop                                                 \n\t"\
"        set     2, %%g4                                             \n\t"\
"        wr      %%g4,0, %%wim                                       \n\t"\
"        nop;nop;nop                                                 \n\t"\
"        mov     %%g3,%%sp                                           \n\t"\
"        mov     %%g4,%%fp                                           \n\t"\
"        jmp     %%g1                                                \n\t"\
"        nop                                                         \n\t"\
: : /* %0 */ "r" (prog)
:       "g1", "g2", "g3", "g4"	\
	);

    
  //prog ();
}

void
clean (paddr, len)
     double *paddr;
     int len;
{
    len >>= 3;
    while (len >= 0)
      {
	  paddr[len] = 0;
	  len--;
      }
}

mmov (raddr, paddr, len)
     int *raddr, *paddr;
     int len;
{
    len >>= 2;
    while (len >= 0)
      {
	  paddr[len] = raddr[len];
	  len--;
      }
}
