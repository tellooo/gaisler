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

#ifndef __BCC_BCC_PARAM_
#define __BCC_BCC_PARAM_

/*
 * This file describes variables which can be used to override the default
 * behavior or BCC. It allows for controlling which UART, timer and interrupt
 * controller to use by BCC. It also allows for disabling AMBA Plug&Play
 * scanning where applicable.
 */
#include <stdint.h>

#include <bcc/bcc.h>

/*
 * Override device probing.
 *
 * Implement these functions to override the default probing/initialization of
 * console, timer and interrupt controller.
 *
 * - __bcc_con_init() should set the global variable __bcc_con_handle.
 * - __bcc_timer_init() should set the global variable __bcc_timer_handle.
 * - __bcc_int_init() should set the global variables __bcc_int_handle and
 *   __bcc_int_irqmp_eirq.
 *
 * These functions shall return BCC_OK on success.
 *
 * On LEON3 systems, the ambapp library is used to probe for AMBA Plug&Play
 * devices. This can be overridden by the user defining one or all of the
 * __bcc_{con,timer,int}_init() functions. If all are overridden, then the
 * ambapp library will not be linked into the binary.
 */
extern int __bcc_con_init(void);
extern int __bcc_timer_init(void);
extern int __bcc_int_init(void);

/*
 * Override default console hardware
 *
 * Set __bcc_con_handle to the address of the UART core to use for stdio. If 0
 * (default) then ambapp is used to scan for the first APBUART on LEON3
 * systems.
 *
 * EXAMPLE:
 * uint32_t __bcc_con_handle = 0xff900000;
 */
extern uint32_t __bcc_con_handle;

/*
 * Override default timer hardware
 *
 * Set __bcc_timer_handle to the address of the timer core to use by BCC. If 0
 * (default) then ambapp is used to scan for the first GPTIMER on LEON3
 * systems.
 *
 * Set __bcc_timer_interrupt to the interrupt number of the first subtimer for
 * __bcc_timer_handle. If __bcc_timer_handle is 0 then ambapp is used to
 * determine the interrupt number on LEON3 systems.
 *
 * EXAMPLE:
 * uint32_t __bcc_timer_handle = 0xff908000;
 * int __bcc_timer_interrupt = 8;
 */
extern uint32_t __bcc_timer_handle;
extern int __bcc_timer_interrupt;

/*
 * Override default interrupt controller hardware
 *
 * Set __bcc_int_handle to the address of the interrupt controller core to use
 * by BCC.  If 0 (default) then ambapp is used to scan for the first IRQMP on
 * LEON3 systems.
 *
 * EXAMPLE:
 * uint32_t __bcc_int_handle = 0xff904000;
 */
extern uint32_t __bcc_int_handle;

/*
 * Override default Extended interrupt number (EIRQ)
 *
 * This applies when the IRQMP interrupt controller is configured with support
 * for extended interrupts.
 *
 * Valid values are (1..15). 0 if extended interrupts are disabled.
 *
 * EXAMPLE:
 * int __bcc_int_irqmp_eirq = 12;
 */
extern int __bcc_int_irqmp_eirq;

/*
 * Override existence of interrupt map
 *
 *  IF __bcc_int_handle == 0 THEN
 *    existence of interrupt map will be probed.
 *  ELSE
 *    IF __bcc_int_irqmp_map == 0 THEN
 *      assume map registers are not available
 *    ELSE
 *      assume map registers are available
 *    ENDIF
 *  ENDIF
 *
 * EXAMPLE:
 * int __bcc_int_irqmp_map = 0;
 */
extern int __bcc_int_irqmp_map;

/*
 * Override number of timestamp register sets
 *
 * This applies when the IRQMP interrupt controller is configured with support
 * for timestamps.
 *
 * EXAMPLE:
 * int __bcc_int_irqmp_nts = 0;
 */
extern int __bcc_int_irqmp_nts;

/*
 * Skip clearing BSS
 *
 * To skip clearing of .bss section during startup, define the global symbol
 * __bcc_cfg_skip_clear_bss. The value does not matter.
 *
 * EXAMPLE:
 * int __bcc_cfg_skip_clear_bss;
 */
extern int __bcc_cfg_skip_clear_bss;

/*
 * Write a character on the console
 *
 * The function shall return 0 on success.
 */
extern int __bcc_con_outbyte(char c);

/*
 * Read the next character from console
 *
 * The returns the read character.
 */
extern char __bcc_con_inbyte(void);

/*
 * Called at start of reset trap before CPU initializations
 *
 * - trap handling is not available
 * - %sp and %fp are not available (do not save/restore)
 * - svt/mvt not configured
 * - .bss not initialized
 */
extern void __bcc_init40(void);

/*
 * Called at start of crt0
 *
 * - trap handling is not available
 * - %sp and %fp are not available (do not save/restore)
 * - .bss not initialized
 * - BCC drivers not initialized
 */
extern void __bcc_init50(void);

/*
 * Called before BCC driver initialization
 *
 * - C runtime is available
 * - BCC drivers are not initialized
 * - Console API, timer API and interrupt API not available
 */
extern void __bcc_init60(void);

/*
 * Called just before main()
 *
 * - Full BCC runtime is available
 */
extern void __bcc_init70(void);

/*
 * Override default heap allocation
 *
 * These parameters affects the behavior of the sbrk() implementation in
 * libbcc. sbrk() is used by the Newlib dynamic memory allocation, for example
 * malloc() and calloc().
 *
 * By default, these values are determined automatically such that the heap
 * starts at end of the .bss section and ends a couple of KiB below the initial
 * stack.
 *
 * The values can be set at run-time but only before dynamic memory functions
 * have been called.
 *
 * __bcc_heap_min: start address of the heap.
 * __bcc_heap_max: end address of the heap.
 *
 * EXAMPLE:
 *   uint8_t *__bcc_heap_min = (uint8_t *) 0x60000000;
 *   uint8_t *__bcc_heap_max = (uint8_t *) 0x70000000;
 */
extern uint8_t *__bcc_heap_min;
extern uint8_t *__bcc_heap_max;

/*
 * Parameters to main(int argc, char *argv[])
 *
 * These variables are given as parameters to main(). Default is 0 and pointer
 * to pointer to NULL.
 */
extern int __bcc_argc;
extern char *((*__bcc_argvp)[]);

#endif

