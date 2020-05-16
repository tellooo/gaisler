/*
 * irqinstall.s for LEON
 */

#include <asm-leon/leon.h>

	/* l0: psr
	   l1: pc
	   l2: npc
	   l3: wim
	   l7: irqnr */	 
         
  	.seg    "text"

        .global locore_readtbr

locore_readtbr:
	retl
	 rd	%tbr,%o0

