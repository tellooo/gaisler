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

#ifndef DRV_GPTIMER_REGS_H_
#define DRV_GPTIMER_REGS_H_

#include <stdint.h>

/* GPTIMER Timer instance */
struct gptimer_timer_regs {
	uint32_t counter;
	uint32_t reload;
	uint32_t ctrl;
	uint32_t latch;
};

/* A GPTIMER can have maximum of 7 subtimers. */
#define GPTIMER_MAX_SUBTIMERS 7

/* GPTIMER common registers */
struct gptimer_regs {
	uint32_t scaler_value;
	uint32_t scaler_reload;
	uint32_t cfg;
	uint32_t latch_cfg;
	struct gptimer_timer_regs timer[GPTIMER_MAX_SUBTIMERS];
};

#define GPTIMER_CTRL_WN         (1 << 7)
#define GPTIMER_CTRL_IP         (1 << 4)
#define GPTIMER_CTRL_IE         (1 << 3)
#define GPTIMER_CTRL_LD         (1 << 2)
#define GPTIMER_CTRL_RS         (1 << 1)
#define GPTIMER_CTRL_EN         (1 << 0)
#define GPTIMER_CFG_EL          (1 << 11)
#define GPTIMER_CFG_DF          (1 << 9)
#define GPTIMER_CFG_SI          (1 << 8)
/* NOTE: IRQ field can not be used with 64-bit interrupt bus. */
#define GPTIMER_CFG_IRQ_BIT     3
#define GPTIMER_CFG_IRQ         (0x1f << GPTIMER_CFG_IRQ_BIT)
#define GPTIMER_CFG_TIMERS      (7 << 0)

#endif
