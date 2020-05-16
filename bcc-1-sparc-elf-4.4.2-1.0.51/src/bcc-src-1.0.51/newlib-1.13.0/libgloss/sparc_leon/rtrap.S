/*
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 * Gaisler Research, Konrad Eisele<eiselekd@web.de>
*/

#include <asm-leon/leonstack.h>
#include <asm-leon/winmacros.h>
                        
/* Registers to not touch at all. */
#define t_psr     l0
#define t_pc      l1
#define t_npc     l2
#define t_wim     l3
#define twin_tmp1 l4
#define glob_tmp  g4
#define curptr    g6

	/* Number of register windows */
	.global _nwindows_min1, _nwindows

        .text
	.align 4
	.globl	leonbare_trapreturn, schedule_callback

leonbare_trapreturn:

        /* a optional scheduler can be called here */
        set schedule_callback, %g2
        ld [%g2], %g2
        cmp %g2,%g0
        beq 3f
         nop
                 
        jmpl %g2,%o7
#ifndef _SOFT_FLOAT
	 add	%sp, FW_REGS_SZ + 8 + SF_REGS_SZ , %o1	! pt_regs ptr
#else
	 add	%sp, SF_REGS_SZ , %o1		   ! pt_regs ptr
#endif
	
3:
	
#ifndef _SOFT_FLOAT
	ld [%sp + (SF_REGS_SZ + PT_REGS_SZ + FW_REGS_SZ - 4)],%g2
	sethi %hi(fpustate_current), %g3 
	st %g2, [%g3+%lo(fpustate_current)]
#endif
		
	wr	%t_psr, 0x0, %psr       ! enable nesting again, clear ET
	
#ifndef _FLAT
	/* Will the rett land us in the invalid window? */
	mov	2, %g1
	sll	%g1, %t_psr, %g1

	sethi %hi(_nwindows), %g2	!NWINDOWS
	ld [%g2+%lo(_nwindows)], %g2
	
       	srl	%g1, %g2, %g2
	or	%g1, %g2, %g1
	rd	%wim, %g2
	andcc	%g2, %g1, %g0
	be	1f		! Nope, just return from the trap
	 sll	%g2, 0x1, %g1

        	/* We have to grab a window before returning. */
		sethi %hi(_nwindows_min1), %g3	!NWINDOWS-1
		ld [%g3+%lo(_nwindows_min1)], %g3

                srl	%g2, %g3,  %g2
                or	%g1, %g2, %g1
                and	%g1, 0xff, %g1

                wr	%g1, 0x0, %wim

        	/* Grrr, make sure we load from the right %sp... */
                PT_LOAD_ALL(sp, t_psr, t_pc, t_npc, g1)
        
                restore	%g0, %g0, %g0
                RW_LOAD(sp)
                b	2f
                 save	%g0, %g0, %g0

	/* Reload the entire frame in case this is from a
	 * kernel system call or whatever...
	 */
1:
#endif
 	PT_LOAD_ALL(sp, t_psr, t_pc, t_npc, g1)
2:

#ifdef _FLAT
	restore
	RW_LOAD(sp)
	save
#endif

	set	_leon_tn0018, %l5
	ld	[%l5], %l5
	cmp	%l5, %g0
	bne	_leon_tn0018_rett
	 nop

	wr	%t_psr, 0x0, %psr
	nop; nop; nop  

	jmp	%t_pc
	rett	%t_npc

#ifdef _FLAT
#warning _FLAT not implemented
#endif
        

