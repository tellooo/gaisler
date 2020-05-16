/*
 * Copyright (c) 2017, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef __BCC_LEON_H_
#define __BCC_LEON_H_

/*
 * Definitions for describing LEON3 hardware
 *
 * This file shall only contain preprocessor #define statements, such that it
 * can be used when preprocessing .S files.
 */

#define ASR17_INDEX_BIT 28
#define ASR17_INDEX     (0xf << ASR17_INDEX_BIT)
#define ASR17_NWIN_BIT  0
#define ASR17_NWIN      (0x1f << ASR17_NWIN_BIT)
#define ASR17_SV_BIT    13
#define ASR17_SV        (1 << ASR17_SV_BIT)
#define ASR17_FPU_BIT   10
#define ASR17_FPU       (0x3 << ASR17_FPU_BIT)
#define ASR17_V8_BIT    8
#define ASR17_V8        (0x01 << ASR17_V8_BIT)

#define ASR17_FPU_NONE  0
#define ASR17_FPU_GRFPU 1
#define ASR17_FPU_MEIKO 2
#define ASR17_FPU_GRFPULITE 3

#define PSR_VER_BIT     24
#define PSR_VER         (0xf << PSR_VER_BIT)
#define PSR_EF          (1 << 12)
#define PSR_S           (1 << 7)
#define PSR_PS          (1 << 6)
#define PSR_ET          (1 << 5)
#define PSR_PIL_BIT     8
#define PSR_PIL         (0xf << PSR_PIL_BIT)
#define PSR_CWP         0x1f

#define TBR_TBA 0xfffff000
#define TBR_TT  0x00000ff0
#define TT_RESET 0x00
#define TT_WINDOW_OVERFLOW 0x05
#define TT_WINDOW_UNDERFLOW 0x06
#define TT_DATA_ACCESS_EXCEPTION 0x09
#define TT_MASK 0xff0
#define TT_SHIFT 4

#define FSR_QNE_BIT     13
#define FSR_QNE         (1 << FSR_QNE_BIT)

#define ASI_MISS 0x01
#define ASI_CTRL 0x02
#define ASI_CTRL_CCR 0x000
#define ASI_ITAG 0x0c
#define ASI_IDATA 0x0d
#define ASI_DTAG 0x0e
#define ASI_DDATA 0x0f
#define ASI_MMUFLUSH 0x18
#define ASI_MMU 0x19
#define ASI_MMU_CTRL 0x000

#define ASI_LEON2_IFLUSH        0x05
#define ASI_LEON2_DFLUSH        0x06
#define ASI_LEON3_IFLUSH        0x10
#define ASI_LEON3_DFLUSH        0x11

/* Cache control register */
#define CCTRL_IP_BIT    15
#define CCTRL_STE (0x1 << 30)
#define CCTRL_RFT (0x1 << 29)
#define CCTRL_PS  (0x1 << 28)
#define CCTRL_TB  (0xf << 24)
#define CCTRL_DS  (0x1 << 23)
#define CCTRL_FD  (0x1 << 22)
#define CCTRL_FI  (0x1 << 21)
#define CCTRL_FT  (0x3 << 19)
#define CCTRL_ST  (0x1 << 17)
#define CCTRL_IB  (0x1 << 16)
#define CCTRL_IP  (0x1 << CCTRL_IP_BIT)
#define CCTRL_DP  (0x1 << 14)
#define CCTRL_ITE (0x3 << 12)
#define CCTRL_IDE (0x3 << 10)
#define CCTRL_DTE (0x3 <<  8)
#define CCTRL_DDE (0x3 <<  6)
#define CCTRL_DF  (0x1 <<  5)
#define CCTRL_IF  (0x1 <<  4)
#define CCTRL_DCS (0x3 <<  2)
#define CCTRL_ICS (0x3 <<  0)

/* Cache Contrl Register */
#define LEON2_CCR               0x80000014

/* Idle register */
#define LEON2_IDLE              0x80000018

/* Product Configuration Register */
#define LEON2_PCR               0x80000024
#define LEON2_PCR_NWIN_BIT      20
#define LEON2_PCR_NWIN          (0x1f << LEON2_PCR_NWIN_BIT)

/*
 * Software trap numbers.
 * Assembly usage: "ta BCC_SW_TRAP_<TYPE>"
 */
#define BCC_SW_TRAP_FLUSH_WINDOWS       0x03
#define BCC_SW_TRAP_SET_PIL             0x09

#if defined(__FIX_B2BST) || defined(__FIX_LEON3FT_B2BST)
  #define B2BSTORE_FIX nop
#else
  #define B2BSTORE_FIX
#endif

#endif

