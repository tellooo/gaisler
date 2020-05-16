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

#ifndef __BCC_BCC_H_
#define __BCC_BCC_H_

#include <stddef.h>
#include <stdint.h>

enum {
        BCC_OK,
        BCC_NOT_AVAILABLE,
        BCC_FAIL
};

/* Return address of trap table address. */
uint32_t bcc_get_trapbase(void);

/*
 * Install trap table entry
 *
 * When this function returns successfully, the current trap table has been
 * updated such that when the trap occurs:
 * - Execution jumps to handler.
 * - %l0: %psr.
 * - %l1: pc
 * - %l2: npc
 * - %l6: tt (0..255) Same value as tt parameter to bcc_set_trap()
 *
 * This function operates on the current trap table. It supports multi vector
 * trapping (MVT) and single vector trapping (SVT).
 *
 * NOTE: The function does not flush CPU instruction cache.
 *
 * tt: Trap type (0..255)
 * handler: Trap handler
 * return:
 * - BCC_OK: success
 * - BCC_FAIL: trap table entry installation failed
 */
int bcc_set_trap(
        int tt,
        void (*handler)(void)
);

/*
 * Enable interrupt based timer service.
 *
 * The function installs a tick interrupt handler which maintains local time
 * using timer hardware. This makes C library / POSIX time functions not
 * limited to hardware constraints anymore.
 *
 * NOTE: Epoch changes to the point in time when bcc_timer_tick_init_period() is
 *       called.
 * NOTE: This function should be called only once.
 * NOTE: The function assumes that the timer (global) scaler is set to 1 MHz.
 *
 * usec_per_tick: Requested timer tick period: number of microseconds per tick.
 *
 * return:
 * - BCC_OK: Success
 * - BCC_FAIL: Failed to enable interrupt based timer service, or already
 *   enabled.
 * - BCC_NOT_AVAILABLE: Hardware or resource not available
 */
int bcc_timer_tick_init_period(
        uint32_t usec_per_tick
);

/* Enable interrupt based timer service with 10 millisecond tick period. */
static inline int bcc_timer_tick_init(void);

/* Return number of microseconds since some point in time. */
uint32_t bcc_timer_get_us(void);

/*
 * Register interrupt handler
 *
 * The function in parameter handler is registered as an interrupt handler for
 * the given interrupt source. The handler is called with arg and source as
 * arguments.
 *
 * Interrupt source is not enabled by this function. bcc_int_unmask() can be
 * used to enable it.
 *
 * Multiple interrupt handlers can be registered for the same interrupt number.
 * They are dispatched at interrupt in the same order as registered.
 *
 * A handler registered with this function should be unregistered with
 * bcc_isr_unregister().
 *
 * source: SPARC interrupt number 1-15 or extended interrupt number 16-31.
 * handler: Pointer to software routine to execute when the interrupt triggers.
 * arg: Passed as parameter to handler.
 *
 * return:
 * - NULL: Indicates failed to install handler.
 * - Pointer: Pointer to ISR handler context. Should not be dereferenced by user.
 *   Used as input to bcc_isr_unregister().
 *
 * NOTE: This function may call malloc().
 */
void *bcc_isr_register(
        int source,
        void (*handler)(
                void *arg,
                int source
        ),
        void *arg
);

/*
 * Unregister interrupt handler
 *
 * It is only allowed to unregister an interrupt handler which has previously
 * been registered with bcc_isr_register().
 *
 * Interrupt source is not disabled by this function. The function
 * bcc_int_mask() can be used to disable it.
 *
 * isr_ctx: ISR handler context returned by bcc_isr_register()
 *
 * return:
 * - BCC_OK: Handler successfully unregistered.
 * - BCC_FAIL: Failed to unregister handler.
 *
 * NOTE: This function may call free().
 */
int bcc_isr_unregister(
        void *isr_ctx
);

/*
 * Interrupt node for registering interrupt handler
 *
 * This structure is used for registering interrupt handlers with
 * bcc_isr_register_node() and bcc_isr_unregister_node(). An application shall
 * not use it if bcc_isr_register() and bcc_isr_unregister() is used.
 */
struct bcc_isr_node {
        /* BCC private data (do not touch) */
        void *__private;
        /* Interrupt source number */
        int source;
        /* User ISR handler */
        void (*handler)(
                void *arg,
                int source
        );
        /* Passed as parameter to handler */
        void *arg;
};

/*
 * Register interrupt handler, non-allocating
 *
 * This function is similar to bcc_isr_register() with the difference that the
 * user is responsible for memory management. It will never call malloc().
 * Instead the caller has to provide a pointer to a preallocated and
 * initialized ISR node of type struct bcc_isr node.
 *
 * The memory pointed to by isr_node shall be considered owned exclusively by
 * the run-time between the call to bcc_isr_register_node() and a future
 * bcc_isr_unregister_node(). It means that the memory must be available for
 * this time and must not be modified by the application. The memory pointed to
 * by isr_node must be writable.
 *
 * This function should be used to install interrupt handlers in applications
 * which want full control over memory allocation.
 *
 * isr_node: Pointer to user initialized ISR node. The fields source, handler
 *           and optionally the arg shall be initialized by the caller.
 *
 * return:
 * - BCC_OK: Handler installed successfully.
 * - BCC_FAIL: Failed to install handler.
 */
int bcc_isr_register_node(
        struct bcc_isr_node *isr_node
);

/*
 * Unregister interrupt handler, non-allocating
 *
 * This function is similar to bcc_isr_unregister() with the difference that
 * the user is responsible for memory management.  It is only allowed to
 * unregister an interrupt handler which has previously been registered with
 * bcc_isr_register_node().
 *
 * isr_node: Same as input parameter to bcc_isr_register_node().
 *
 * return:
 * - BCC_OK: Handler successfully unregistered.
 * - BCC_FAIL: Failed to unregister handler.
 */
int bcc_isr_unregister_node(
        const struct bcc_isr_node *isr_node
);

int bcc_isr_route(int source, int vsrc);

/*
 * Disable all maskable interrupts and return the previous interrupt
 * enable/disable state
 *
 * A matching bcc_int_enable() with the return value as parameter must be
 * called to exit the interrupt disabled state. It is allowed to do nested
 * calls to bcc_int_disable(), and if so the same number of bcc_int_enable()
 * must be called.
 *
 * This function modifies the SPARC V8 PSR.PIL field. Interrupt controller is
 * not touched.
 *
 * return: Previous interrupt level (used when calling bcc_int_enable().
 */
static inline int bcc_int_disable(void);

/*
 * Return to a previous interrupt enable/disable state
 *
 * The plevel parameter is the return value from a previous call to
 * bcc_int_disable(). At return, interrupts may be enabled or disabled
 * depending on plevel.
 *
 * This function modifies the SPARC V8 PSR.PIL field. Interrupt controller is
 * not touched.
 *
 * plevel: The interrupt protection level to set. Must be the return value from
 *         the most recent call to bcc_int_disable().
 */
static inline void bcc_int_enable(int plevel);

/*
 * Mask (disable) an interrupt source on the current CPU.
 *
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: device not available
 */
int bcc_int_mask(int source);

/*
 * Unmask (enable) an interrupt source on the current CPU.
 *
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: device not available
 */
int bcc_int_unmask(int source);

/*
 * Clear an interrupt source.
 *
 * source: Interrupt source 1..31
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: device not available
 */
int bcc_int_clear(int source);

/*
 * Force an interrupt level on the current CPU.
 *
 * level: Interrupt Request Level 1..15
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: device not available
 */
int bcc_int_force(int level);

/*
 * Make an interrupt source pending.
 *
 * source: SPARC interrupt number 1-15 or extended interrupt number 16-31.
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: device not available
 */
int bcc_int_pend(int source);

/*
 * Set mapping from bus interrupt line to interrupt controller interrupt line.
 *
 * busintline: Bus interrupt line number 0..63
 * irqmpintline: Interrupt controller interrupt line 1..31
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: Device or functionality not available
 */
int bcc_int_map_set(int busintline, int irqmpintline);

/*
 * Get mapping from bus interrupt line to interrupt controller interrupt line.
 *
 * busintline: Bus interrupt line number 0..63
 * return:
 * - 1..31: Interrupt controller interrupt line 1..31
 * -   -1:  Device or functionality not available
 */
int bcc_int_map_get(int busintline);

/*
 * Get current interrupt nest count
 *
 * return:
 * - 0: not in interrupt context
 * - 1: first interrupt context level
 * - n: n:th interrupt context level
 */
int bcc_int_nestcount(void);

/*
 * Disable interrupt nesting
 *
 * After calling this function, PSR.PIL will be raised to 0xf (highest) when
 * an interrupt occurs on any level.
 *
 * NOTE: Nested interrupts are disabled by default.
 *
 * return: BCC_OK
 */
int bcc_int_disable_nesting(void);

/*
 * Enable interrupt nested
 *
 * After calling this function, PSR.PIL will be raised to the current
 * interrupt level when an interrupt occurs.
 *
 * return: BCC_OK
 */
int bcc_int_enable_nesting(void);

/*
 * Configure interrupt nesting
 *
 * Configures in detail how the SPARC processor interrupt level is set when an
 * interrupt occurs. After calling this function, PSR.PIL will be raised to
 * newpil when an interrupt occurs on level pil.
 *
 * NOTE: newpil must be equal to or greater than pil.
 *
 * return:
 * - BCC_OK: success
 * - BCC_FAIL: illegal parameters
 */
int bcc_int_set_nesting(int pil, int newpil);

/*
 * Atomic Load-Store Unsigned Byte
 *
 * Copy a byte from addr to temporary variable, then rewrite the addressed byte
 * in memory with 0xff.
 *
 * Algorithm:
 *   tmp := *addr;
 *   *addr := 0xff;
 *   return tmp;
 *
 * addr: Address of byte to operate on.
 * return: Value at addr before store.
 */
uint8_t bcc_ldstub(uint8_t *addr);

/*
 * Get ID of the current processor.
 *
 * The first processor in the system has ID 0.
 *
 * return: ID of the current processor
 */
int bcc_get_cpuid(void);

/*
 * Set 64-bit words to zero
 *
 * This function sets n 64-bit words to zero, starting at address dst. All
 * writes are performed with the SPARC V8 std instruction.
 *
 * dst: Start address of area to set to zero. Must be aligned to a 64-bit word.
 * n: Number of 64-bit words to set to zero.
 */
void bcc_dwzero(uint64_t *dst, size_t n);

void bcc_flush_cache(void);
void bcc_flush_icache(void);
void bcc_flush_dcache(void);

/*
 * Cache control register access
 *
 * For bit masks, see the CCTRL_ definitions in bcc/leon.h.
 */
void bcc_set_ccr(uint32_t data);
uint32_t bcc_get_ccr(void);

/* Load with forced cache miss*/
uint32_t bcc_loadnocache(uint32_t *addr);
uint16_t bcc_loadnocache16(uint16_t *addr);
uint8_t bcc_loadnocache8(uint8_t *addr);

/*
 * Set processor interrupt level atomically
 *
 * This function is implemented as a software trap and guarantees atomic update
 * of PSR.PIL.
 *
 * newpil: New value for PSR.PIL (0..15)
 * return: Old value of PSR.PIL
 */
int bcc_set_pil(int newpil);

/*
 * Get processor interrupt level
 *
 * return: Value of PSR.PIL
 */
int bcc_get_pil(void);

void bcc_set_psr(uint32_t psr);
uint32_t bcc_get_psr(void);
void bcc_set_tbr(uint32_t tbr);
uint32_t bcc_get_tbr(void);

/*
 * Get number of processors in the system
 *
 * NOTE: This function will return -1 if the run-time is not aware of the
 * interrupt controller.
 *
 * return: Number of processor in the system or -1 if unknown.
 */
int bcc_get_cpu_count(void);

/*
 * Start a processor
 *
 * cpuid: The processor to start (0 .. (get_cpu_count()-1))
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: processor or device not available
 */
int bcc_start_processor(int cpuid);

/*
 * Force an interrupt level on a processor
 *
 * level: Interrupt Request Level 1..15
 * cpuid: Processor to interrupt
 * return:
 * - BCC_OK: success
 * - BCC_NOT_AVAILABLE: processor or device not available
 */
int bcc_send_interrupt(int level, int cpuid);

/*
 * Power down current processor
 *
 * return: BCC_OK
 */
int bcc_power_down(void);

/* Floating point context */
struct bcc_fpu_state {
        uint64_t d[16];
        uint32_t fsr;
        uint32_t _align;
};

/*
 * Save floating-point context
 *
 * This function can generate traps fp_disabled and fp_exception.
 *
 * state: location to save FPU context. Should point to preallocated struct
 *         bcc_fpu_state, aligned to 8 byte.
 * return: BCC_OK
 */
int bcc_fpu_save(struct bcc_fpu_state *state);

/*
 * Restore floating-point context
 *
 * This function can generate traps fp_disabled and fp_exception.
 *
 * state: location to restore FPU context from. Should point to struct
 *        bcc_fpu_state, aligned to 8 byte.
 * return: BCC_OK
 */
int bcc_fpu_restore(struct bcc_fpu_state *state);

/* Bring in implementation of inline functions. */
#include "bcc_inline.h"

#endif

