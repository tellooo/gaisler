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

#ifndef DRV_CLKGATE_BITS_H
#define DRV_CLKGATE_BITS_H

/* Masks applicable to GR712RC */
#define CLKGATE_GR712RC_B1553BRM        (1 << 11)
#define CLKGATE_GR712RC_GRTC            (1 << 10)
#define CLKGATE_GR712RC_GRTM            (1 <<  9)
#define CLKGATE_GR712RC_PROPRIETARY     (1 <<  8)
#define CLKGATE_GR712RC_CAN             (1 <<  7)
#define CLKGATE_GR712RC_GRSPW5          (1 <<  6)
#define CLKGATE_GR712RC_GRSPW4          (1 <<  5)
#define CLKGATE_GR712RC_GRSPW3          (1 <<  4)
#define CLKGATE_GR712RC_GRSPW2          (1 <<  3)
#define CLKGATE_GR712RC_GRSPW1          (1 <<  2)
#define CLKGATE_GR712RC_GRSPW0          (1 <<  1)
#define CLKGATE_GR712RC_GRETH           (1 <<  0)
#define CLKGATE_GR712RC_ALL             (0xfff)

/* Masks applicable to GR740 */
#define CLKGATE_GR740_MCTRL             (1 << 10)
#define CLKGATE_GR740_SPI               (1 <<  9)
#define CLKGATE_GR740_UART1             (1 <<  8)
#define CLKGATE_GR740_UART0             (1 <<  7)
#define CLKGATE_GR740_L4STAT            (1 <<  6)
#define CLKGATE_GR740_CAN               (1 <<  5)
#define CLKGATE_GR740_GR1553B           (1 <<  4)
#define CLKGATE_GR740_PCI               (1 <<  3)
#define CLKGATE_GR740_SPACEWIRE         (1 <<  2)
#define CLKGATE_GR740_GRETH1            (1 <<  1)
#define CLKGATE_GR740_GRETH0            (1 <<  0)
#define CLKGATE_GR740_ALL               (0x7ff)

/* GR716 "Clock gating unit (Primary)" */
#define CLKGATE0_GR716_MCTRL1           (1 << 30)
#define CLKGATE0_GR716_SPI4S            (1 << 29)
#define CLKGATE0_GR716_SPWTDP0          (1 << 28)
#define CLKGATE0_GR716_BO0              (1 << 27)
#define CLKGATE0_GR716_MEMPROT0         (1 << 26)
#define CLKGATE0_GR716_AHBUART          (1 << 25)
#define CLKGATE0_GR716_L3STAT0          (1 << 24)
#define CLKGATE0_GR716_BO               (1 << 23)
#define CLKGATE0_GR716_UART5            (1 << 21)
#define CLKGATE0_GR716_UART4            (1 << 20)
#define CLKGATE0_GR716_UART3            (1 << 19)
#define CLKGATE0_GR716_UART2            (1 << 18)
#define CLKGATE0_GR716_UART1            (1 << 17)
#define CLKGATE0_GR716_UART0            (1 << 16)
#define CLKGATE0_GR716_GRPWM1           (1 << 15)
#define CLKGATE0_GR716_GRPWM0           (1 << 14)
#define CLKGATE0_GR716_GRDACADC         (1 << 13)
#define CLKGATE0_GR716_I2CSLV1          (1 << 12)
#define CLKGATE0_GR716_I2CSLV0          (1 << 11)
#define CLKGATE0_GR716_I2CMST1          (1 << 10)
#define CLKGATE0_GR716_I2CMST0          (1 <<  9)
#define CLKGATE0_GR716_SPI1             (1 <<  8)
#define CLKGATE0_GR716_SPI0             (1 <<  7)
#define CLKGATE0_GR716_SPIM1            (1 <<  6)
#define CLKGATE0_GR716_SPIM0            (1 <<  5)
#define CLKGATE0_GR716_MCTRL0           (1 <<  4)
#define CLKGATE0_GR716_GRPWTX           (1 <<  3)
#define CLKGATE0_GR716_GRPWRX           (1 <<  2)
#define CLKGATE0_GR716_I2C2AHB          (1 <<  1)
#define CLKGATE0_GR716_SPI2AHB          (1 <<  0)

/* GR716 "Clock gating unit (Secondary)" */
#define CLKGATE1_GR716_GPIOSEQ1         (1 << 21)
#define CLKGATE1_GR716_GPIOSEQ0         (1 << 20)
#define CLKGATE1_GR716_ADC7             (1 << 19)
#define CLKGATE1_GR716_ADC6             (1 << 18)
#define CLKGATE1_GR716_ADC5             (1 << 17)
#define CLKGATE1_GR716_ADC4             (1 << 16)
#define CLKGATE1_GR716_ADC3             (1 << 15)
#define CLKGATE1_GR716_ADC2             (1 << 14)
#define CLKGATE1_GR716_ADC1             (1 << 13)
#define CLKGATE1_GR716_ADC0             (1 << 12)
#define CLKGATE1_GR716_DAC3             (1 << 11)
#define CLKGATE1_GR716_DAC2             (1 << 10)
#define CLKGATE1_GR716_DAC1             (1 <<  9)
#define CLKGATE1_GR716_DAC0             (1 <<  8)
#define CLKGATE1_GR716_GRSPW            (1 <<  7)
#define CLKGATE1_GR716_GRCAN1           (1 <<  6)
#define CLKGATE1_GR716_GRCAN0           (1 <<  5)
#define CLKGATE1_GR716_GR1553B          (1 <<  4)
#define CLKGATE1_GR716_GRDMAC3          (1 <<  3)
#define CLKGATE1_GR716_GRDMAC2          (1 <<  2)
#define CLKGATE1_GR716_GRDMAC1          (1 <<  1)
#define CLKGATE1_GR716_GRDMAC0          (1 <<  0)

#endif

