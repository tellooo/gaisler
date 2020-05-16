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

#ifndef _FTMCTRL_REGS_H_
#define _FTMCTRL_REGS_H_

#include <stdint.h>

struct ftmctrl_regs {
	uint32_t mcfg1;
	uint32_t mcfg2;
	uint32_t mcfg3;
	uint32_t mcfg4;
};

#define FTMCTRL_MCFG1_IOEN      (1 << 19)
#define FTMCTRL_MCFG1_PWEN      (1 << 11)

#define FTMCTRL_MCFG3_RSE       (1 << 28)
#define FTMCTRL_MCFG3_WB        (1 << 11)
#define FTMCTRL_MCFG3_RB        (1 << 10)
#define FTMCTRL_MCFG3_RE        (1 << 9)
#define FTMCTRL_MCFG3_PE        (1 << 8)
#define FTMCTRL_MCFG3_TCB       (0xff << 0)

#define FTMCTRL_MCFG4_WB        (1 << 16)
#define FTMCTRL_MCFG4_TCB       (0xffff << 0)

#endif
