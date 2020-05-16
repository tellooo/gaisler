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

#ifndef DRV_APBUART_REGS_H
#define DRV_APBUART_REGS_H

/*
 * Register description for APBUART - AMBA APB UART Serial Interface
 */

#include <stdint.h>

/* APBUART registers
 *
 * Offset | Name   | Description
 * ------ | ------ | ----------------------------------------
 * 0x0000 | data   | UART data register
 * 0x0004 | status | UART status register
 * 0x0008 | ctrl   | UART control register
 * 0x000c | scaler | UART scaler register
 * 0x0010 | debug  | UART FIFO debug register
 */

struct apbuart_regs {
  /** @brief UART data register
   *
   * Bit    | Name   | Description
   * ------ | ------ | ----------------------------------------
   * 7-0    | data   | Holding register or FIFO
   */
        uint32_t data;          /* 0x0000 */

  /** @brief UART status register
   *
   * Bit    | Name   | Description
   * ------ | ------ | ----------------------------------------
   * 31-26  | RCNT   | Receiver FIFO count
   * 25-20  | TCNT   | Transmitter FIFO count
   * 10     | RF     | Receiver FIFO full
   * 9      | TF     | Transmitter FIFO full
   * 8      | RH     | Receiver FIFO half-full
   * 7      | TH     | Transmitter FIFO half-full
   * 6      | FE     | Framing error
   * 5      | PE     | Parity error
   * 4      | OV     | Overrun
   * 3      | BR     | Break received
   * 2      | TE     | Transmitter FIFO empty
   * 1      | TS     | Transmitter shift register empty
   * 0      | DR     | Data ready
   */
        uint32_t status;        /* 0x0004 */

  /** @brief UART control register
   *
   * Bit    | Name   | Description
   * ------ | ------ | ----------------------------------------
   * 31     | FA     | FIFOs available
   * 14     | SI     | Transmitter shift register empty interrupt enable
   * 13     | DI     | Delayed interrupt enable
   * 12     | BI     | Break interrupt enable
   * 11     | DB     | FIFO debug mode enable
   * 10     | RF     | Receiver FIFO interrupt enable
   * 9      | TF     | Transmitter FIFO interrupt enable
   * 8      | EC     | External clock
   * 7      | LB     | Loop back
   * 6      | FL     | Flow control
   * 5      | PE     | Parity enable
   * 4      | PS     | Parity select
   * 3      | TI     | Transmitter interrupt enable
   * 2      | RI     | Receiver interrupt enable
   * 1      | TE     | Transmitter enable
   * 0      | RE     | Receiver enable
   */
        uint32_t ctrl;          /* 0x0008 */

  /** @brief UART scaler register
   *
   * Bit    | Name   | Description
   * ------ | ------ | ----------------------------------------
   * 11-0   | RELOAD | Scaler reload value
   */
        uint32_t scaler;        /* 0x000c */

  /** @brief UART FIFO debug register
   *
   * Bit    | Name   | Description
   * ------ | ------ | ----------------------------------------
   * 7-0    | data   | Holding register or FIFO
   */
        uint32_t debug;         /* 0x0010 */

};

/* APBUART register bits. */

/* Control register */
#define APBUART_CTRL_FA         (1 << 31)
#define APBUART_CTRL_DB         (1 << 11)
#define APBUART_CTRL_RF         (1 << 10)
#define APBUART_CTRL_TF         (1 << 9)
#define APBUART_CTRL_LB         (1 << 7)
#define APBUART_CTRL_FL         (1 << 6)
#define APBUART_CTRL_PE         (1 << 5)
#define APBUART_CTRL_PS         (1 << 4)
#define APBUART_CTRL_TI         (1 << 3)
#define APBUART_CTRL_RI         (1 << 2)
#define APBUART_CTRL_TE         (1 << 1)
#define APBUART_CTRL_RE         (1 << 0)

/* Status register */
#define APBUART_STATUS_RF       (1 << 10)
#define APBUART_STATUS_TF       (1 << 9)
#define APBUART_STATUS_RH       (1 << 8)
#define APBUART_STATUS_TH       (1 << 7)
#define APBUART_STATUS_FE       (1 << 6)
#define APBUART_STATUS_PE       (1 << 5)
#define APBUART_STATUS_OV       (1 << 4)
#define APBUART_STATUS_BR       (1 << 3)
#define APBUART_STATUS_TE       (1 << 2)
#define APBUART_STATUS_TS       (1 << 1)
#define APBUART_STATUS_DR       (1 << 0)

/* For APBUART implemented without FIFO */
#define APBUART_STATUS_HOLD_REGISTER_EMPTY (1 << 2)

#endif

