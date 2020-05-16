/*
 *  Idle Thread Body
 *
 *  This routine puts LEON3 in power-down mode.
 *
 *  COPYRIGHT (c) 2004.
 *  Gaisler Research.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *
 *  $Id: ithread.S,v 1.2 2006/01/09 10:41:21 ralf Exp $
 */


	
#include <rtems/asm.h>
	
/* IDLE BODY: LEON3 specific power-down function */

        .align 4
        PUBLIC(_BSP_Thread_Idle_body)
SYM(_BSP_Thread_Idle_body):
pwdloop:  mov   %g0, %asr19
	  lda	[%sp] 1, %g0		! Needed for UT699 and GR712
          ba    pwdloop
          nop
          retl 
          nop

