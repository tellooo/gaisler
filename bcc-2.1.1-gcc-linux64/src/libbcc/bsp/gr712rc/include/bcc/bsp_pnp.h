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

/* AMBA Plug&Play information for GR712RC */

#define GAISLER_AHBJTAG_0_PNP_VERSION                      0

#define GAISLER_AHBSTAT_0_PNP_APB                          0x80000f00
#define GAISLER_AHBSTAT_0_PNP_APB_MASK                     0x00000100
#define GAISLER_AHBSTAT_0_PNP_APB_IRQ                      1
#define GAISLER_AHBSTAT_0_PNP_VERSION                      0

#define GAISLER_APBMST_0_PNP_AHB_0                         0x80000000
#define GAISLER_APBMST_0_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_0_PNP_VERSION                       0

#define GAISLER_APBMST_1_PNP_AHB_0                         0x80100000
#define GAISLER_APBMST_1_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_1_PNP_VERSION                       0

#define GAISLER_APBUART_0_PNP_APB                          0x80000100
#define GAISLER_APBUART_0_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_0_PNP_APB_IRQ                      2
#define GAISLER_APBUART_0_PNP_VERSION                      1

#define GAISLER_APBUART_1_PNP_APB                          0x80100100
#define GAISLER_APBUART_1_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_1_PNP_APB_IRQ                      17
#define GAISLER_APBUART_1_PNP_VERSION                      1

#define GAISLER_APBUART_2_PNP_APB                          0x80100200
#define GAISLER_APBUART_2_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_2_PNP_APB_IRQ                      18
#define GAISLER_APBUART_2_PNP_VERSION                      1

#define GAISLER_APBUART_3_PNP_APB                          0x80100300
#define GAISLER_APBUART_3_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_3_PNP_APB_IRQ                      19
#define GAISLER_APBUART_3_PNP_VERSION                      1

#define GAISLER_APBUART_4_PNP_APB                          0x80100400
#define GAISLER_APBUART_4_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_4_PNP_APB_IRQ                      20
#define GAISLER_APBUART_4_PNP_VERSION                      1

#define GAISLER_APBUART_5_PNP_APB                          0x80100500
#define GAISLER_APBUART_5_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_5_PNP_APB_IRQ                      21
#define GAISLER_APBUART_5_PNP_VERSION                      1

#define GAISLER_ASCS_0_PNP_APB                             0x80000700
#define GAISLER_ASCS_0_PNP_APB_MASK                        0x00000100
#define GAISLER_ASCS_0_PNP_APB_IRQ                         16
#define GAISLER_ASCS_0_PNP_VERSION                         0

#define GAISLER_B1553BRM_0_PNP_AHB_0                       0xfff00000
#define GAISLER_B1553BRM_0_PNP_AHB_0_MASK                  0x00001000
#define GAISLER_B1553BRM_0_PNP_AHB_IRQ                     14
#define GAISLER_B1553BRM_0_PNP_VERSION                     0

#define GAISLER_CANAHB_0_PNP_AHB_0                         0xfff30000
#define GAISLER_CANAHB_0_PNP_AHB_0_MASK                    0x00001000
#define GAISLER_CANAHB_0_PNP_AHB_IRQ                       5
#define GAISLER_CANAHB_0_PNP_VERSION                       1

#define GAISLER_CANMUX_0_PNP_APB                           0x80000500
#define GAISLER_CANMUX_0_PNP_APB_MASK                      0x00000100
#define GAISLER_CANMUX_0_PNP_VERSION                       0

#define GAISLER_CLKGATE_0_PNP_APB                          0x80000d00
#define GAISLER_CLKGATE_0_PNP_APB_MASK                     0x00000100
#define GAISLER_CLKGATE_0_PNP_VERSION                      0

#define GAISLER_ETHMAC_0_PNP_APB                           0x80000e00
#define GAISLER_ETHMAC_0_PNP_APB_MASK                      0x00000100
#define GAISLER_ETHMAC_0_PNP_APB_IRQ                       14
#define GAISLER_ETHMAC_0_PNP_VERSION                       0

#define GAISLER_FTAHBRAM_0_PNP_AHB_0                       0xa0000000
#define GAISLER_FTAHBRAM_0_PNP_AHB_0_MASK                  0x00100000
#define GAISLER_FTAHBRAM_0_PNP_APB                         0x80100000
#define GAISLER_FTAHBRAM_0_PNP_APB_MASK                    0x00000100
#define GAISLER_FTAHBRAM_0_PNP_VERSION                     18

#define GAISLER_FTMCTRL_0_PNP_AHB_0                        0x00000000
#define GAISLER_FTMCTRL_0_PNP_AHB_0_MASK                   0x20000000
#define GAISLER_FTMCTRL_0_PNP_AHB_1                        0x20000000
#define GAISLER_FTMCTRL_0_PNP_AHB_1_MASK                   0x20000000
#define GAISLER_FTMCTRL_0_PNP_AHB_2                        0x40000000
#define GAISLER_FTMCTRL_0_PNP_AHB_2_MASK                   0x40000000
#define GAISLER_FTMCTRL_0_PNP_APB                          0x80000000
#define GAISLER_FTMCTRL_0_PNP_APB_MASK                     0x00000100
#define GAISLER_FTMCTRL_0_PNP_VERSION                      1

#define GAISLER_GPIO_0_PNP_APB                             0x80000900
#define GAISLER_GPIO_0_PNP_APB_MASK                        0x00000100
#define GAISLER_GPIO_0_PNP_VERSION                         0

#define GAISLER_GPIO_1_PNP_APB                             0x80000a00
#define GAISLER_GPIO_1_PNP_APB_MASK                        0x00000100
#define GAISLER_GPIO_1_PNP_VERSION                         0

#define GAISLER_GPREG_0_PNP_APB                            0x80000600
#define GAISLER_GPREG_0_PNP_APB_MASK                       0x00000100
#define GAISLER_GPREG_0_PNP_VERSION                        0

#define GAISLER_GPTIMER_0_PNP_APB                          0x80000300
#define GAISLER_GPTIMER_0_PNP_APB_MASK                     0x00000100
#define GAISLER_GPTIMER_0_PNP_APB_IRQ                      8
#define GAISLER_GPTIMER_0_PNP_VERSION                      0

#define GAISLER_GRTC_0_PNP_AHB_0                           0xfff10000
#define GAISLER_GRTC_0_PNP_AHB_0_MASK                      0x00000100
#define GAISLER_GRTC_0_PNP_AHB_IRQ                         14
#define GAISLER_GRTC_0_PNP_VERSION                         2

#define GAISLER_GRTIMER_0_PNP_APB                          0x80100600
#define GAISLER_GRTIMER_0_PNP_APB_MASK                     0x00000100
#define GAISLER_GRTIMER_0_PNP_APB_IRQ                      7
#define GAISLER_GRTIMER_0_PNP_VERSION                      1

#define GAISLER_GRTM_0_PNP_APB                             0x80000b00
#define GAISLER_GRTM_0_PNP_APB_MASK                        0x00000100
#define GAISLER_GRTM_0_PNP_APB_IRQ                         29
#define GAISLER_GRTM_0_PNP_VERSION                         0

#define GAISLER_I2CMST_0_PNP_APB                           0x80000c00
#define GAISLER_I2CMST_0_PNP_APB_MASK                      0x00000100
#define GAISLER_I2CMST_0_PNP_APB_IRQ                       28
#define GAISLER_I2CMST_0_PNP_VERSION                       1

#define GAISLER_IRQMP_0_PNP_APB                            0x80000200
#define GAISLER_IRQMP_0_PNP_APB_MASK                       0x00000100
#define GAISLER_IRQMP_0_PNP_VERSION                        3

#define GAISLER_LEON3DSU_0_PNP_AHB_0                       0x90000000
#define GAISLER_LEON3DSU_0_PNP_AHB_0_MASK                  0x10000000
#define GAISLER_LEON3DSU_0_PNP_VERSION                     1

#define GAISLER_LEON3FT_0_PNP_VERSION                      0

#define GAISLER_LEON3FT_1_PNP_VERSION                      0

#define GAISLER_SATCAN_0_PNP_AHB_0                         0xfff20000
#define GAISLER_SATCAN_0_PNP_AHB_0_MASK                    0x00000100
#define GAISLER_SATCAN_0_PNP_AHB_IRQ                       14
#define GAISLER_SATCAN_0_PNP_VERSION                       0

#define GAISLER_SLINK_0_PNP_APB                            0x80000800
#define GAISLER_SLINK_0_PNP_APB_MASK                       0x00000100
#define GAISLER_SLINK_0_PNP_APB_IRQ                        13
#define GAISLER_SLINK_0_PNP_VERSION                        1

#define GAISLER_SPICTRL_0_PNP_APB                          0x80000400
#define GAISLER_SPICTRL_0_PNP_APB_MASK                     0x00000100
#define GAISLER_SPICTRL_0_PNP_APB_IRQ                      13
#define GAISLER_SPICTRL_0_PNP_VERSION                      2

#define GAISLER_SPW2_0_PNP_MST_IRQ                         22
#define GAISLER_SPW2_0_PNP_APB                             0x80100800
#define GAISLER_SPW2_0_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_0_PNP_APB_IRQ                         22
#define GAISLER_SPW2_0_PNP_VERSION                         0

#define GAISLER_SPW2_1_PNP_MST_IRQ                         23
#define GAISLER_SPW2_1_PNP_APB                             0x80100900
#define GAISLER_SPW2_1_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_1_PNP_APB_IRQ                         23
#define GAISLER_SPW2_1_PNP_VERSION                         0

#define GAISLER_SPW2_2_PNP_MST_IRQ                         24
#define GAISLER_SPW2_2_PNP_APB                             0x80100a00
#define GAISLER_SPW2_2_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_2_PNP_APB_IRQ                         24
#define GAISLER_SPW2_2_PNP_VERSION                         0

#define GAISLER_SPW2_3_PNP_MST_IRQ                         25
#define GAISLER_SPW2_3_PNP_APB                             0x80100b00
#define GAISLER_SPW2_3_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_3_PNP_APB_IRQ                         25
#define GAISLER_SPW2_3_PNP_VERSION                         0

#define GAISLER_SPW2_4_PNP_MST_IRQ                         26
#define GAISLER_SPW2_4_PNP_APB                             0x80100c00
#define GAISLER_SPW2_4_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_4_PNP_APB_IRQ                         26
#define GAISLER_SPW2_4_PNP_VERSION                         0

#define GAISLER_SPW2_5_PNP_MST_IRQ                         27
#define GAISLER_SPW2_5_PNP_APB                             0x80100d00
#define GAISLER_SPW2_5_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_5_PNP_APB_IRQ                         27
#define GAISLER_SPW2_5_PNP_VERSION                         0

#endif /* __BCC_BSP_PNP_H_ */

