/*
 * Copyright (c) 2019, Cobham Gaisler AB
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

/*
 * ROM resident initialization for BCC 2.0.x and 2.1.x.
 *
 * THIS FILE CONTAINS EXAMPLE CODE. THE CODE IN THIS FILE IS NOT TESTED OR
 * VALIDATED
 *
 * Example link:
 * sparc-gaisler-elf-gcc -qbsp=leon3 -mcpu=leon3 -qnano -c init40.S -o init40.o
 * sparc-gaisler-elf-gcc -qbsp=leon3 -mcpu=leon3 -qnano -O2 -T linkcmds-rom main.c init40.o
 *
 * Author: Martin Åberg, Cobham Gaisler AB, 2019
 */

/*
 * system configuration parameters
 * configuration is customized for the UT700 LEAP board
 */
#define CFG_SYSFREQ_HZ          (125 * 1000 * 1000)

#define CFG_RAM_START           0x40000000
#define CFG_RAM_SIZE            (32 * 1024 * 1024)

#define CFG_MCTRL0_ADDR         0x80000000
#define CFG_APBUART0_ADDR       0x80000100
#define CFG_IRQMP0_ADDR         0x80000200
#define CFG_GPTIMER0_ADDR       0x80000300

/* example configuration for 32 MiB SDRAM (32-bit) */
/* NOTE: leap does not have checkbits on SDRAM data bus */
#define CFG_MCTRL0_MCFG1        0x0803c2ff
#define CFG_MCTRL0_MCFG2        0xf5a0600a
#define CFG_MCTRL0_MCFG3        0x083cd000

#define CFG_APBUART0_SCALER     0x00000196

#include <bcc/leon.h>

.macro FUNC_BEGIN name
        .global \name
        .align 4
        .type \name, #function
        \name:
.endm

.macro FUNC_END name
        .size \name, .-\name
.endm

FUNC_BEGIN __bcc_init50
        retl
         nop
FUNC_END __bcc_init50

FUNC_BEGIN __bcc_init40
/* we use g7 as return address and make sure it is preserved */
        mov     %o7, %g7
        clr     %g6
        clr     %g5

/* initialize psr, wim, tbr */
/* Do not enable traps because we cannot handle them yet. */
        set     (PSR_EF | PSR_PIL | PSR_S | PSR_PS), %g2
        wr      %g2, %psr
        nop
        nop
        nop
/* really init, will be updated later before traps are enabled */
        wr      %g0, %tbr
        wr      %g0, %wim
        wr      %g0, %asr17

/*
 * Early initialization of CPU private cache control registers: enable data
 * cache snoop and instruction burst fetch. Leave data and instruction caches
 * disabled.
 */
        set     (CCTRL_DS | CCTRL_FD | CCTRL_FI | CCTRL_IB), %g2
        sta     %g2, [%g0] ASI_CTRL

/* Disable MMU and flush TLB caches. */
        sta     %g0, [%g0] ASI_MMU
        sta     %g0, [%g0] ASI_MMUFLUSH

/* initialize iu registers in all register windows */
        clr     %g2
        call    iu_init
         nop

        /* Make early init and tests trap on window overflow/underflow */
        wr      2, %wim

/* initialize fpu registers, including fsr */
        call    fpu_init
         nop

/* optional: clock gate peripherals not used */

/* optional: set watch dog */

/* flush and enable instruction cache */
        set     (CCTRL_DS | CCTRL_FD | CCTRL_FI | CCTRL_IB | CCTRL_ICS), %g2
        sta     %g2, [%g0] ASI_CTRL

/* init irqmp by disabling all interrupts */
        set     CFG_IRQMP0_ADDR, %g1
        st      %g0, [%g1 + 0x00]
        st      %g0, [%g1 + 0x04]
        st      %g0, [%g1 + 0x08]

/* set mctrl->{mcfg1,mcfg2,mcfg3} */
        set     CFG_MCTRL0_ADDR, %g1
        set     CFG_MCTRL0_MCFG1, %g2
        st      %g2, [%g1 + 0x00]
        set     CFG_MCTRL0_MCFG2, %g2
        st      %g2, [%g1 + 0x04]
        set     CFG_MCTRL0_MCFG3, %g2
        st      %g2, [%g1 + 0x08]

/* set gptimer0->subtimer0 scaler for 1 us tick and enable subtimer0. */
        set     CFG_GPTIMER0_ADDR, %g1
        set     (CFG_SYSFREQ_HZ / (1000 * 1000)) - 1, %g2
        /* scaler reload and value */
        st      %g2, [%g1 + 0x04]
        st      %g2, [%g1 + 0x00]

        set     0xffffffff, %g2
        /* subtimer0 count and reload */
        st      %g2, [%g1 + 0x10]
        st      %g2, [%g1 + 0x14]
        /* enable subtimer0 */
        set     7, %g2
        st      %g2, [%g1 + 0x18]

/* optional: enable uart0 tx/rx and set scaler */
        set     CFG_APBUART0_ADDR, %g1
        set     CFG_APBUART0_SCALER, %g2
        st      %g2, [%g1 + 0x0c]
        /* enable uart tx/rx and clear status */
        set     3, %g2
        st      %g2, [%g1 + 0x08]
        st      %g0, [%g1 + 0x04]

/* set all of RAM to known value using st or std instruction (bcc_dwzero) */
        set     CFG_RAM_START, %o0
        set     (CFG_RAM_SIZE / 8), %o1
        call    bcc_dwzero
         nop

/* flush and enable data cache */
        set     (CCTRL_DS | CCTRL_FD | CCTRL_FI | CCTRL_IB | CCTRL_ICS), %g2
        sta     %g2, [%g0] ASI_CTRL

/* set stack pointer to top of RAM */
        set     (CFG_RAM_START + CFG_RAM_SIZE), %sp

        jmpl    %g7+8, %g0
         nop
FUNC_END __bcc_init40


#ifndef ASR17_V8
#define ASR17_V8 0x100
#endif

/*
 * Initialize IU general-purpose registers.
 *
 * Global registers and window registers in all windows
 * are initialized.
 *
 * - %g2 is not touched
 * - %g5, %g6 and %g7 are not touched
 * - %y is initialized if available.
 * It is assumed that WIM register is 0 at entry.
 */
FUNC_BEGIN iu_init
        /*
         * Save function return address while initializing window registers.
         * This will be restored just before return.
         */
        mov     %o7, %g4

        /*
         * The LEON3/LEON4 configuration register provides information on how
         * various configuration options were set during synthesis.
         */
        rd      %asr17, %g1

        /* V8 bit indicates existence of the %y register. */
        andcc   %g1, ASR17_V8, %g0
        beq     .Lskipy
         nop
        wr      %g0, %y
.Lskipy:
        /* NWIN field is the number of implemented register windows - 1. */
        and     %g1, ASR17_NWIN, %g1
        add     %g1, 1, %g1
1:
        clr     %i0
        clr     %i1
        clr     %i2
        clr     %i3
        clr     %i4
        clr     %i5
        clr     %i6
        clr     %i7
        clr     %l0
        clr     %l1
        clr     %l2
        clr     %l3
        clr     %l4
        clr     %l5
        clr     %l6
        clr     %l7
        save
        subcc   %g1, 1, %g1
        bne     1b
         nop

        clr     %g3

        /* Restore return address */
        mov     %g4, %o7
        clr     %g4
        retl
         clr    %o7
FUNC_END iu_init


FUNC_BEGIN fpu_init
        /* Enable FPU */
        rd      %psr, %o5
        set     PSR_EF, %g1
        or      %o5, %g1, %g1
        wr      %g1, %psr
        /* PSR is not ready until after third instruction after wrpsr. */
        nop
        nop

        /* Initialize FPU control/status registers. */
        set     init_zerofp, %g1
        ld      [%g1], %fsr
        /* Initialize FPU general-purpose registers */
        ldd     [%g1], %f0
        ldd     [%g1], %f2
        ldd     [%g1], %f4
        ldd     [%g1], %f6
        ldd     [%g1], %f8
        ldd     [%g1], %f10
        ldd     [%g1], %f12
        ldd     [%g1], %f14
        ldd     [%g1], %f16
        ldd     [%g1], %f18
        ldd     [%g1], %f20
        ldd     [%g1], %f22
        ldd     [%g1], %f24
        ldd     [%g1], %f26
        ldd     [%g1], %f28
        ldd     [%g1], %f30

        /* Restore PSR */
        wr      %o5, %psr
        nop
        nop
        nop
        retl
         nop
FUNC_END fpu_init

        .section ".rodata"
        .align 8
init_zerofp:
        .word 0, 0

