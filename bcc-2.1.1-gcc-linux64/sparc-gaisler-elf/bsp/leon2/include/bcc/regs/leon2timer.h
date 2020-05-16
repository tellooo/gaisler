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

#ifndef __LEON2TIMER_REGS_H_
#define __LEON2TIMER_REGS_H_

/*
 * Register description for LEON2 timer
 */

#include <stdint.h>

/* LEON2 subtimer instance */
struct leon2timer_timer_regs {
        uint32_t counter;
        uint32_t reload;
        uint32_t ctrl;
        uint32_t _reserved;
};

/* A LEON2 starts with 2 subtimers. */
#define LEON2TIMER_NSUBTIMERS 2

/* LEON2 timer common registers */
struct leon2timer_regs {
        struct leon2timer_timer_regs timer[LEON2TIMER_NSUBTIMERS];
        uint32_t scaler_value;
        uint32_t scaler_reload;
};

/* NOTE: There are no IE or IP bits. */
#define LEON2TIMER_CTRL_LD      (1 << 2)
#define LEON2TIMER_CTRL_RL      (1 << 1)
#define LEON2TIMER_CTRL_EN      (1 << 0)

/* The prescaler is 10 bit wide. */
#define LEON2TIMER_SCALER_MASK  (0x3ff)

#endif

