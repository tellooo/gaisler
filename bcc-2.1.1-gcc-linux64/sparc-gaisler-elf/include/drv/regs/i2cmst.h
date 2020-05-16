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

#ifndef DRV_I2CMST_REGS_H
#define DRV_I2CMST_REGS_H

#include <stdint.h>

struct i2cmst_regs {
        uint32_t prescale;              /* 0x00 */
        uint32_t ctrl;                  /* 0x04 */
        union {
                uint32_t tx;            /* 0x08 Write only */
                const uint32_t rx;      /* 0x08 Read only */
        } x;
        union {
                uint32_t cmd;           /* 0x0C Write only */
                const uint32_t status;  /* 0x0C Read only */
        } c;
        uint32_t filter;                /* 0x10 */
};

/* Control register */
#define I2CMST_CTRL_EN_BIT      7
#define I2CMST_CTRL_IEN_BIT     6

#define I2CMST_CTRL_EN          (1 << I2CMST_CTRL_EN_BIT)
#define I2CMST_CTRL_IEN         (1 << I2CMST_CTRL_IEN_BIT)

/* Prescale register */
#define I2CMST_PRESCALE_DIV_BIT 1

#define I2CMST_PRESCALE_DIV     (0xFFFF << I2CMST_PRESCALE_DIVISOR_BIT)

/* Transmitt register */
#define I2CMST_TX_TDATA_BIT     1
#define I2CMST_TX_WR_BIT        0

#define I2CMST_TX_TDATA         (0x7F << I2CMST_TX_TDATA_BIT)
#define I2CMST_TX_WR            (1 << I2CMST_TX_WR_BIT)

/* Command register */
#define I2CMST_CMD_STA_BIT      7
#define I2CMST_CMD_STO_BIT      6
#define I2CMST_CMD_RD_BIT       5
#define I2CMST_CMD_WR_BIT       4
#define I2CMST_CMD_ACK_BIT      3
#define I2CMST_CMD_IACK_BIT     0

#define I2CMST_CMD_STA          (1 << I2CMST_CMD_STA_BIT)
#define I2CMST_CMD_STO          (1 << I2CMST_CMD_STO_BIT)
#define I2CMST_CMD_RD           (1 << I2CMST_CMD_RD_BIT)
#define I2CMST_CMD_WR           (1 << I2CMST_CMD_WR_BIT)
#define I2CMST_CMD_ACK          (1 << I2CMST_CMD_ACK_BIT)
#define I2CMST_CMD_IACK         (1 << I2CMST_CMD_IACK_BIT)

/* Status register */
#define I2CMST_STATUS_RXACK_BIT 7
#define I2CMST_STATUS_BUSY_BIT  6
#define I2CMST_STATUS_AL_BIT    5
#define I2CMST_STATUS_TIP_BIT   1
#define I2CMST_STATUS_IF_BIT    0

#define I2CMST_STATUS_RXACK     (1 << I2CMST_STATUS_RXACK_BIT)
#define I2CMST_STATUS_BUSY      (1 << I2CMST_STATUS_BUSY_BIT)
#define I2CMST_STATUS_AL        (1 << I2CMST_STATUS_AL_BIT)
#define I2CMST_STATUS_TIP       (1 << I2CMST_STATUS_TIP_BIT)
#define I2CMST_STATUS_IF        (1 << I2CMST_STATUS_IF_BIT)

#endif

