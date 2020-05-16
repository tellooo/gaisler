/*

    LEON2/3 LIBIO low-level routines 
    Written by Jiri Gaisler.
    Copyright (C) 2004  Gaisler Research AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * regwin.s for LEON
 */

#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>
        
/* Store the register window onto the 8-byte aligned area starting
 * at %reg.  It might be %sp, it might not, we don't care.
 */
#define RW_STORE(reg) \
	std	%l0, [%reg + RW_L0]; \
	B2BSTORE_FIX; \
	std	%l2, [%reg + RW_L2]; \
	B2BSTORE_FIX; \
	std	%l4, [%reg + RW_L4]; \
	B2BSTORE_FIX; \
	std	%l6, [%reg + RW_L6]; \
	B2BSTORE_FIX; \
	std	%i0, [%reg + RW_I0]; \
	B2BSTORE_FIX; \
	std	%i2, [%reg + RW_I2]; \
	B2BSTORE_FIX; \
	std	%i4, [%reg + RW_I4]; \
	B2BSTORE_FIX; \
	std	%i6, [%reg + RW_I6]; \
	B2BSTORE_FIX;

/* Load a register window from the area beginning at %reg. */
#define RW_LOAD(reg) \
	ldd	[%reg + RW_L0], %l0; \
	ldd	[%reg + RW_L2], %l2; \
	ldd	[%reg + RW_L4], %l4; \
	ldd	[%reg + RW_L6], %l6; \
	ldd	[%reg + RW_I0], %i0; \
	ldd	[%reg + RW_I2], %i2; \
	ldd	[%reg + RW_I4], %i4; \
	ldd	[%reg + RW_I6], %i6;

/* Loading and storing struct pt_reg trap frames. */
#define PT_LOAD_INS(base_reg) \
        ldd     [%base_reg + SF_REGS_SZ + PT_I0], %i0; \
        ldd     [%base_reg + SF_REGS_SZ + PT_I2], %i2; \
        ldd     [%base_reg + SF_REGS_SZ + PT_I4], %i4; \
        ldd     [%base_reg + SF_REGS_SZ + PT_I6], %i6;

#define PT_LOAD_GLOBALS(base_reg) \
        ld      [%base_reg + SF_REGS_SZ + PT_G1], %g1; \
        ldd     [%base_reg + SF_REGS_SZ + PT_G2], %g2; \
        ldd     [%base_reg + SF_REGS_SZ + PT_G4], %g4; \
        ldd     [%base_reg + SF_REGS_SZ + PT_G6], %g6;

#define PT_LOAD_GLOBALS_23(base_reg) \
        ldd     [%base_reg + SF_REGS_SZ + PT_G2], %g2; 
        
#define PT_LOAD_YREG(base_reg, scratch) \
        ld      [%base_reg + SF_REGS_SZ + PT_Y], %scratch; \
        wr      %scratch, 0x0, %y;

#define PT_LOAD_PRIV(base_reg, pt_psr, pt_pc, pt_npc) \
        ld      [%base_reg + SF_REGS_SZ + PT_PSR], %pt_psr; \
        ld      [%base_reg + SF_REGS_SZ + PT_PC], %pt_pc; \
        ld      [%base_reg + SF_REGS_SZ + PT_NPC], %pt_npc;

#define PT_LOAD_ALL(base_reg, pt_psr, pt_pc, pt_npc, scratch) \
        PT_LOAD_YREG(base_reg, scratch) \
        PT_LOAD_INS(base_reg) \
        PT_LOAD_GLOBALS(base_reg) \
        PT_LOAD_PRIV(base_reg, pt_psr, pt_pc, pt_npc)

#define PT_LOAD_ALL_FAST(base_reg, pt_psr, pt_pc, pt_npc, scratch) \
        PT_LOAD_YREG(base_reg, scratch) \
        PT_LOAD_GLOBALS(base_reg) 

#define PT_STORE_INS(base_reg) \
	B2BSTORE_FIX; \
        std     %i0, [%base_reg + SF_REGS_SZ + PT_I0]; \
	B2BSTORE_FIX; \
        std     %i2, [%base_reg + SF_REGS_SZ + PT_I2]; \
	B2BSTORE_FIX; \
        std     %i4, [%base_reg + SF_REGS_SZ + PT_I4]; \
	B2BSTORE_FIX; \
        std     %i6, [%base_reg + SF_REGS_SZ + PT_I6]; \
	B2BSTORE_FIX;

#define PT_STORE_GLOBALS(base_reg) \
	B2BSTORE_FIX; \
        st      %g1, [%base_reg + SF_REGS_SZ + PT_G1]; \
        std     %g2, [%base_reg + SF_REGS_SZ + PT_G2]; \
	B2BSTORE_FIX; \
        std     %g4, [%base_reg + SF_REGS_SZ + PT_G4]; \
	B2BSTORE_FIX; \
        std     %g6, [%base_reg + SF_REGS_SZ + PT_G6]; \
	B2BSTORE_FIX;

#define PT_STORE_YREG(base_reg, scratch) \
        rd      %y, %scratch; \
        st      %scratch, [%base_reg + SF_REGS_SZ + PT_Y]; \
	B2BSTORE_FIX;

#define PT_STORE_PRIV(base_reg, pt_psr, pt_pc, pt_npc) \
	B2BSTORE_FIX; \
        st      %pt_psr, [%base_reg + SF_REGS_SZ + PT_PSR]; \
        st      %pt_pc,  [%base_reg + SF_REGS_SZ + PT_PC]; \
        st      %pt_npc, [%base_reg + SF_REGS_SZ + PT_NPC]; \
	B2BSTORE_FIX;

#define PT_STORE_ALL(base_reg, reg_psr, reg_pc, reg_npc, g_scratch) \
        PT_STORE_PRIV(base_reg, reg_psr, reg_pc, reg_npc) \
        PT_STORE_GLOBALS(base_reg) \
        PT_STORE_YREG(base_reg, g_scratch) \
        PT_STORE_INS(base_reg)

#define PT_STORE_ALL_FAST(base_reg, reg_psr, reg_pc, reg_npc, g_scratch) \
        PT_STORE_GLOBALS(base_reg) \
        PT_STORE_YREG(base_reg, g_scratch) 

/* Store the fpu register window*/
#define FW_STORE(reg) \
        std	%f0, [reg + FW_F0]; \
	B2BSTORE_FIX; \
	std	%f2, [reg + FW_F2]; \
	B2BSTORE_FIX; \
	std	%f4, [reg + FW_F4]; \
	B2BSTORE_FIX; \
	std	%f6, [reg + FW_F6]; \
	B2BSTORE_FIX; \
	std	%f8, [reg + FW_F8]; \
	B2BSTORE_FIX; \
	std	%f10, [reg + FW_F10]; \
	B2BSTORE_FIX; \
	std	%f12, [reg + FW_F12]; \
	B2BSTORE_FIX; \
	std	%f14, [reg + FW_F14]; \
	B2BSTORE_FIX; \
	std	%f16, [reg + FW_F16]; \
	B2BSTORE_FIX; \
	std	%f18, [reg + FW_F18]; \
	B2BSTORE_FIX; \
	std	%f20, [reg + FW_F20]; \
	B2BSTORE_FIX; \
	std	%f22, [reg + FW_F22]; \
	B2BSTORE_FIX; \
	std	%f24, [reg + FW_F24]; \
	B2BSTORE_FIX; \
	std	%f26, [reg + FW_F26]; \
	B2BSTORE_FIX; \
	std	%f28, [reg + FW_F28]; \
	B2BSTORE_FIX; \
	std	%f30, [reg + FW_F30]; \
	B2BSTORE_FIX; \
	st	%fsr, [reg + FW_FSR]; \
	B2BSTORE_FIX;

/* Load a fpu register window from the area beginning at reg. */
#define FW_LOAD(reg) \
        ldd	[reg + FW_F0], %f0; \
	ldd	[reg + FW_F2], %f2; \
	ldd	[reg + FW_F4], %f4; \
	ldd	[reg + FW_F6], %f6; \
	ldd	[reg + FW_F8], %f8; \
	ldd	[reg + FW_F10], %f10; \
	ldd	[reg + FW_F12], %f12; \
	ldd	[reg + FW_F14], %f14; \
	ldd	[reg + FW_F16], %f16; \
	ldd	[reg + FW_F18], %f18; \
	ldd	[reg + FW_F20], %f20; \
	ldd	[reg + FW_F22], %f22; \
	ldd	[reg + FW_F24], %f24; \
	ldd	[reg + FW_F26], %f26; \
	ldd	[reg + FW_F28], %f28; \
	ldd	[reg + FW_F30], %f30; \
	ld	[reg + FW_FSR], %fsr;

#define SET_WIM_CWPMIN2(psr_reg,tmp1,tmp2,tmp3,tmp4) \
        sethi	%hi(_nwindows_min2), %##tmp1; \
	and	%##psr_reg, SPARC_PSR_WIN_MASK, %##tmp3; \
	mov	1, %##tmp2; \
	ld	[ %##tmp1 + %lo(_nwindows_min2)], %##tmp1; \
        sll	%##tmp2, %##tmp3, %##tmp3; \
        sll	%##tmp3, 2, %##tmp4; \
        srl	%##tmp3, %##tmp1, %##tmp1; \
        or	%##tmp4, %##tmp1, %##tmp3; \
	wr      %##tmp3, 0x0, %wim; \
        nop; nop; nop;

#define SET_WIM_CWPMIN1(psr_reg,tmp1,tmp2,tmp3,tmp4) \
        sethi	%hi(_nwindows_min1), %##tmp1; \
	and	%##psr_reg, SPARC_PSR_WIN_MASK, %##tmp3; \
	mov	1, %##tmp2; \
	ld	[ %##tmp1 + %lo(_nwindows_min1)], %##tmp1; \
        sll	%##tmp2, %##tmp3, %##tmp3; \
        sll	%##tmp3, 1, %##tmp4; \
        srl	%##tmp3, %##tmp1, %##tmp1; \
        or	%##tmp4, %##tmp1, %##tmp3; \
	wr      %##tmp3, 0x0, %wim; \
        nop; nop; nop;
