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

#ifndef __BCC_BSP_PNP_H_
#define __BCC_BSP_PNP_H_

/* AMBA Plug&Play information for AGGA4 */

#define ESA_AHBSTAT_0_PNP_APB                              0x8000000c
#define ESA_AHBSTAT_0_PNP_APB_MASK                         0x00000008
#define ESA_AHBSTAT_0_PNP_VERSION                          0

#define ESA_CFG_0_PNP_APB                                  0x80000024
#define ESA_CFG_0_PNP_APB_MASK                             0x00000004
#define ESA_CFG_0_PNP_VERSION                              0

#define ESA_CIC_0_PNP_APB                                  0x80000100
#define ESA_CIC_0_PNP_APB_MASK                             0x0000000c
#define ESA_CIC_0_PNP_VERSION                              0

#define ESA_CRC_0_PNP_APB                                  0x80000120
#define ESA_CRC_0_PNP_APB_MASK                             0x00000028
#define ESA_CRC_0_PNP_VERSION                              0

#define ESA_DSUUART_0_PNP_APB                              0x800000c4
#define ESA_DSUUART_0_PNP_APB_MASK                         0x00000010
#define ESA_DSUUART_0_PNP_VERSION                          0

#define ESA_EEPROM_0_PNP_APB                               0x800001f8
#define ESA_EEPROM_0_PNP_APB_MASK                          0x00000004
#define ESA_EEPROM_0_PNP_VERSION                           0

#define ESA_FFT_0_PNP_AHB_0                                0xb0000000
#define ESA_FFT_0_PNP_AHB_0_MASK                           0x10000000
#define ESA_FFT_0_PNP_VERSION                              0

#define ESA_GNSS_0_PNP_AHB_0                               0xa0000000
#define ESA_GNSS_0_PNP_AHB_0_MASK                          0x10000000
#define ESA_GNSS_0_PNP_VERSION                             0

#define ESA_IO_0_PNP_APB                                   0x800000e0
#define ESA_IO_0_PNP_APB_MASK                              0x0000000c
#define ESA_IO_0_PNP_VERSION                               0

#define ESA_IP1553B_0_PNP_APB                              0x80000140
#define ESA_IP1553B_0_PNP_APB_MASK                         0x00000024
#define ESA_IP1553B_0_PNP_VERSION                          0

#define ESA_IRQ_0_PNP_APB                                  0x80000090
#define ESA_IRQ_0_PNP_APB_MASK                             0x00000010
#define ESA_IRQ_0_PNP_VERSION                              0

#define ESA_LEON2APB_0_PNP_AHB_0                           0x80000000
#define ESA_LEON2APB_0_PNP_AHB_0_MASK                      0x10000000
#define ESA_LEON2APB_0_PNP_VERSION                         0

#define ESA_LEON2_0_PNP_APB                                0x80000014
#define ESA_LEON2_0_PNP_APB_MASK                           0x00000008
#define ESA_LEON2_0_PNP_VERSION                            0

#define ESA_MCTRL_0_PNP_AHB_0                              0x00000000
#define ESA_MCTRL_0_PNP_AHB_0_MASK                         0x20000000
#define ESA_MCTRL_0_PNP_AHB_1                              0x20000000
#define ESA_MCTRL_0_PNP_AHB_1_MASK                         0x20000000
#define ESA_MCTRL_0_PNP_AHB_2                              0x40000000
#define ESA_MCTRL_0_PNP_AHB_2_MASK                         0x40000000
#define ESA_MCTRL_0_PNP_APB                                0x80000000
#define ESA_MCTRL_0_PNP_APB_MASK                           0x0000000c
#define ESA_MCTRL_0_PNP_VERSION                            0

#define ESA_PIO_0_PNP_APB                                  0x800000a0
#define ESA_PIO_0_PNP_APB_MASK                             0x0000000c
#define ESA_PIO_0_PNP_VERSION                              0

#define ESA_PLL_0_PNP_APB                                  0x800001e0
#define ESA_PLL_0_PNP_APB_MASK                             0x00000010
#define ESA_PLL_0_PNP_VERSION                              0

#define ESA_RESET_0_PNP_APB                                0x800000ec
#define ESA_RESET_0_PNP_APB_MASK                           0x0000000c
#define ESA_RESET_0_PNP_VERSION                            0

#define ESA_SGPO_0_PNP_APB                                 0x80000170
#define ESA_SGPO_0_PNP_APB_MASK                            0x00000010
#define ESA_SGPO_0_PNP_VERSION                             0

#define ESA_SPI_0_PNP_APB                                  0x80000110
#define ESA_SPI_0_PNP_APB_MASK                             0x00000010
#define ESA_SPI_0_PNP_VERSION                              0

#define ESA_SPWMOD_0_PNP_APB                               0x80000300
#define ESA_SPWMOD_0_PNP_APB_MASK                          0x00000018
#define ESA_SPWMOD_0_PNP_VERSION                           0

#define ESA_SPW_0_PNP_APB                                  0x80000200
#define ESA_SPW_0_PNP_APB_MASK                             0x00000040
#define ESA_SPW_0_PNP_VERSION                              0

#define ESA_SPW_1_PNP_APB                                  0x80000240
#define ESA_SPW_1_PNP_APB_MASK                             0x00000040
#define ESA_SPW_1_PNP_VERSION                              0

#define ESA_SPW_2_PNP_APB                                  0x80000280
#define ESA_SPW_2_PNP_APB_MASK                             0x00000040
#define ESA_SPW_2_PNP_VERSION                              0

#define ESA_SPW_3_PNP_APB                                  0x800002c0
#define ESA_SPW_3_PNP_APB_MASK                             0x00000040
#define ESA_SPW_3_PNP_VERSION                              0

#define ESA_TIMER_0_PNP_APB                                0x80000040
#define ESA_TIMER_0_PNP_APB_MASK                           0x00000050
#define ESA_TIMER_0_PNP_VERSION                            0

#define ESA_UARTMOD_0_PNP_APB                              0x800001c8
#define ESA_UARTMOD_0_PNP_APB_MASK                         0x00000004
#define ESA_UARTMOD_0_PNP_VERSION                          0

#define ESA_UART_0_PNP_APB                                 0x80000180
#define ESA_UART_0_PNP_APB_MASK                            0x00000024
#define ESA_UART_0_PNP_VERSION                             0

#define ESA_UART_1_PNP_APB                                 0x800001a4
#define ESA_UART_1_PNP_APB_MASK                            0x00000024
#define ESA_UART_1_PNP_VERSION                             0

#define ESA_VERSION_0_PNP_APB                              0x800000fc
#define ESA_VERSION_0_PNP_APB_MASK                         0x00000004
#define ESA_VERSION_0_PNP_VERSION                          0

#define ESA_WDOG_0_PNP_APB                                 0x800000b4
#define ESA_WDOG_0_PNP_APB_MASK                            0x00000010
#define ESA_WDOG_0_PNP_VERSION                             0

#define ESA_WPROT2_0_PNP_APB                               0x800000d0
#define ESA_WPROT2_0_PNP_APB_MASK                          0x00000010
#define ESA_WPROT2_0_PNP_VERSION                           0

#define ESA_WPROT_0_PNP_APB                                0x8000001c
#define ESA_WPROT_0_PNP_APB_MASK                           0x00000008
#define ESA_WPROT_0_PNP_VERSION                            0

#define GAISLER_LEON2DSU_0_PNP_AHB_0                       0x90000000
#define GAISLER_LEON2DSU_0_PNP_AHB_0_MASK                  0x10000000
#define GAISLER_LEON2DSU_0_PNP_VERSION                     0

#endif /* __BCC_BSP_PNP_H_ */

