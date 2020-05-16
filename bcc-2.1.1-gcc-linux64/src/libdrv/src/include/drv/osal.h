/*
 * Copyright (c) 2018, Cobham Gaisler AB
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

#ifndef DRV_OSAL_H
#define DRV_OSAL_H

#include <stdint.h>
#include <drv/osal_cfg.h>
#include <drv/drvret.h>

/* Prototype of service routine for the driver library */
typedef void (*osal_isr_t)(void *);

/* Backend resources for maintaining ISR registration. Allocated by driver. */
struct osal_isr_ctx;

/*
 * Register interrupt handler
 *
 * The function in parameter handler is registered as the interrupt handler for
 * the given interrupt source. The handler is called with arg as argument.
 * Interrupt source is enabled in the interrupt controller.
 *
 * source: Interrupt number.
 * handler: Pointer to software routine to execute when the interrupt triggers.
 * arg: Passed as parameter to handler.
 *
 * return:
 * - DRV_OK: Handler installed successfully.
 * - DRV_INVALID_SOURCE: Invalid source parameter.
 * - DRV_FAIL: Unknown failure.
 */
int osal_isr_register(
        struct osal_isr_ctx *ctx,
        int source,
        osal_isr_t handler,
        void *arg
);

/*
 * Unregister interrupt handler
 *
 * Unregister interrupt handler.  The parameters source, handler and arg must
 * be the same as used when the handler was registered with osal_isr_register.
 * It is only allowed to unregister an interrupt which has previously been
 * registered with osal_isr_register.
 *
 * source: Interrupt number.
 * handler: Pointer to software routine previously installed.
 * arg: Parameter value previously installed.
 *
 * return:
 * - DRV_OK: Handler successfully unregistered.
 * - DRV_INVALID_SOURCE: Invalid source parameter. Handler is not unregistered.
 * - DRV_FAIL: Unknown failure. Handler is not unregistered.
 */
int osal_isr_unregister(
        struct osal_isr_ctx *ctx,
        int source,
        osal_isr_t handler,
        void *arg
);

/*
 * Atomic Load-Store Unsigned Byte
 *
 * Copy a byte from addr to temporary variable, then rewrite the addressed byte
 * in memory with 0xff.
 *
 * In case atomic operation is required, the SPARC "ldstub" instruction can be
 * used. Otherwise an emulating function is enough.
 *
 * Algorithm:
 *   tmp := *addr;
 *   *addr := 0xff;
 *   return tmp;
 *
 * addr: Address of byte to operate on.
 *
 * return: Value at addr before store.
 */
uint8_t osal_ldstub(uint8_t *addr);

/*
 * Return peripheral bus frequency in Hz.
 */
unsigned long osal_busfreq(void);

#endif

