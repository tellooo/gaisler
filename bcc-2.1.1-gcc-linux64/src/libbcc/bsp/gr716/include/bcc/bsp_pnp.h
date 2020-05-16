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

/* AMBA Plug&Play information for GR716 */

#define GAISLER_ADC_0_PNP_APB                              0x80400000
#define GAISLER_ADC_0_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_0_PNP_APB_IRQ                          28
#define GAISLER_ADC_0_PNP_VERSION                          0

#define GAISLER_ADC_1_PNP_APB                              0x80401000
#define GAISLER_ADC_1_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_1_PNP_APB_IRQ                          29
#define GAISLER_ADC_1_PNP_VERSION                          0

#define GAISLER_ADC_2_PNP_APB                              0x80402000
#define GAISLER_ADC_2_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_2_PNP_APB_IRQ                          30
#define GAISLER_ADC_2_PNP_VERSION                          0

#define GAISLER_ADC_3_PNP_APB                              0x80403000
#define GAISLER_ADC_3_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_3_PNP_APB_IRQ                          31
#define GAISLER_ADC_3_PNP_VERSION                          0

#define GAISLER_ADC_4_PNP_APB                              0x80404000
#define GAISLER_ADC_4_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_4_PNP_APB_IRQ                          32
#define GAISLER_ADC_4_PNP_VERSION                          0

#define GAISLER_ADC_5_PNP_APB                              0x80405000
#define GAISLER_ADC_5_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_5_PNP_APB_IRQ                          33
#define GAISLER_ADC_5_PNP_VERSION                          0

#define GAISLER_ADC_6_PNP_APB                              0x80406000
#define GAISLER_ADC_6_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_6_PNP_APB_IRQ                          34
#define GAISLER_ADC_6_PNP_VERSION                          0

#define GAISLER_ADC_7_PNP_APB                              0x80407000
#define GAISLER_ADC_7_PNP_APB_MASK                         0x00000100
#define GAISLER_ADC_7_PNP_APB_IRQ                          35
#define GAISLER_ADC_7_PNP_VERSION                          0

#define GAISLER_AHB2AHB_0_PNP_AHB_0                        0x00000000
#define GAISLER_AHB2AHB_0_PNP_AHB_0_MASK                   0x20000000
#define GAISLER_AHB2AHB_0_PNP_AHB_1                        0x40000000
#define GAISLER_AHB2AHB_0_PNP_AHB_1_MASK                   0x20000000
#define GAISLER_AHB2AHB_0_PNP_AHB_2                        0x80000000
#define GAISLER_AHB2AHB_0_PNP_AHB_2_MASK                   0x10000000
#define GAISLER_AHB2AHB_0_PNP_AHB_3                        0xf0000000
#define GAISLER_AHB2AHB_0_PNP_AHB_3_MASK                   0x10000000
#define GAISLER_AHB2AHB_0_PNP_VERSION                      2

#define GAISLER_AHB2AHB_1_PNP_AHB_0                        0x90000000
#define GAISLER_AHB2AHB_1_PNP_AHB_0_MASK                   0x10000000
#define GAISLER_AHB2AHB_1_PNP_VERSION                      2

#define GAISLER_AHB2AHB_2_PNP_AHB_0                        0x30000000
#define GAISLER_AHB2AHB_2_PNP_AHB_0_MASK                   0x00100000
#define GAISLER_AHB2AHB_2_PNP_AHB_1                        0x90000000
#define GAISLER_AHB2AHB_2_PNP_AHB_1_MASK                   0x10000000
#define GAISLER_AHB2AHB_2_PNP_VERSION                      2

#define GAISLER_AHB2AHB_3_PNP_AHB_0                        0x00000000
#define GAISLER_AHB2AHB_3_PNP_AHB_0_MASK                   0x80000000
#define GAISLER_AHB2AHB_3_PNP_AHB_1                        0x80000000
#define GAISLER_AHB2AHB_3_PNP_AHB_1_MASK                   0x10000000
#define GAISLER_AHB2AHB_3_PNP_AHB_2                        0xf0000000
#define GAISLER_AHB2AHB_3_PNP_AHB_2_MASK                   0x10000000
#define GAISLER_AHB2AHB_3_PNP_VERSION                      2

#define GAISLER_AHBROM_0_PNP_AHB_0                         0x00000000
#define GAISLER_AHBROM_0_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_AHBROM_0_PNP_VERSION                       0

#define GAISLER_AHBSTAT_0_PNP_APB                          0x8000a000
#define GAISLER_AHBSTAT_0_PNP_APB_MASK                     0x00000100
#define GAISLER_AHBSTAT_0_PNP_APB_IRQ                      63
#define GAISLER_AHBSTAT_0_PNP_VERSION                      0

#define GAISLER_AHBSTAT_1_PNP_APB                          0x80306000
#define GAISLER_AHBSTAT_1_PNP_APB_MASK                     0x00000100
#define GAISLER_AHBSTAT_1_PNP_APB_IRQ                      63
#define GAISLER_AHBSTAT_1_PNP_VERSION                      0

#define GAISLER_AHBTRACE_0_PNP_AHB_0                       0x9ff20000
#define GAISLER_AHBTRACE_0_PNP_AHB_0_MASK                  0x00020000
#define GAISLER_AHBTRACE_0_PNP_VERSION                     0

#define GAISLER_AHBUART_0_PNP_APB                          0x8000f000
#define GAISLER_AHBUART_0_PNP_APB_MASK                     0x00000100
#define GAISLER_AHBUART_0_PNP_VERSION                      0

#define GAISLER_AHBUART_1_PNP_APB                          0x94000000
#define GAISLER_AHBUART_1_PNP_APB_MASK                     0x00000100
#define GAISLER_AHBUART_1_PNP_VERSION                      0

#define GAISLER_APBMST_0_PNP_AHB_0                         0x80000000
#define GAISLER_APBMST_0_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_0_PNP_VERSION                       1

#define GAISLER_APBMST_1_PNP_AHB_0                         0x80100000
#define GAISLER_APBMST_1_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_1_PNP_VERSION                       1

#define GAISLER_APBMST_2_PNP_AHB_0                         0x80300000
#define GAISLER_APBMST_2_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_2_PNP_VERSION                       1

#define GAISLER_APBMST_3_PNP_AHB_0                         0x80400000
#define GAISLER_APBMST_3_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_3_PNP_VERSION                       1

#define GAISLER_APBMST_4_PNP_AHB_0                         0x94000000
#define GAISLER_APBMST_4_PNP_AHB_0_MASK                    0x00100000
#define GAISLER_APBMST_4_PNP_VERSION                       1

#define GAISLER_APBUART_0_PNP_APB                          0x80300000
#define GAISLER_APBUART_0_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_0_PNP_APB_IRQ                      24
#define GAISLER_APBUART_0_PNP_VERSION                      1

#define GAISLER_APBUART_1_PNP_APB                          0x80301000
#define GAISLER_APBUART_1_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_1_PNP_APB_IRQ                      25
#define GAISLER_APBUART_1_PNP_VERSION                      1

#define GAISLER_APBUART_2_PNP_APB                          0x80302000
#define GAISLER_APBUART_2_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_2_PNP_APB_IRQ                      42
#define GAISLER_APBUART_2_PNP_VERSION                      1

#define GAISLER_APBUART_3_PNP_APB                          0x80303000
#define GAISLER_APBUART_3_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_3_PNP_APB_IRQ                      44
#define GAISLER_APBUART_3_PNP_VERSION                      1

#define GAISLER_APBUART_4_PNP_APB                          0x80304000
#define GAISLER_APBUART_4_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_4_PNP_APB_IRQ                      45
#define GAISLER_APBUART_4_PNP_VERSION                      1

#define GAISLER_APBUART_5_PNP_APB                          0x80305000
#define GAISLER_APBUART_5_PNP_APB_MASK                     0x00000100
#define GAISLER_APBUART_5_PNP_APB_IRQ                      46
#define GAISLER_APBUART_5_PNP_VERSION                      1

#define GAISLER_CLKGATE_0_PNP_APB                          0x80006000
#define GAISLER_CLKGATE_0_PNP_APB_MASK                     0x00000100
#define GAISLER_CLKGATE_0_PNP_VERSION                      1

#define GAISLER_CLKGATE_1_PNP_APB                          0x80007000
#define GAISLER_CLKGATE_1_PNP_APB_MASK                     0x00000100
#define GAISLER_CLKGATE_1_PNP_VERSION                      1

#define GAISLER_BANDGAP_0_PNP_APB                          0x8010b000
#define GAISLER_BANDGAP_0_PNP_APB_MASK                     0x00000100
#define GAISLER_BANDGAP_0_PNP_APB_IRQ                      63
#define GAISLER_BANDGAP_0_PNP_VERSION                      0

#define GAISLER_BO_0_PNP_APB                               0x8010c000
#define GAISLER_BO_0_PNP_APB_MASK                          0x00000100
#define GAISLER_BO_0_PNP_APB_IRQ                           63
#define GAISLER_BO_0_PNP_VERSION                           0

#define GAISLER_DAC_0_PNP_APB                              0x80408000
#define GAISLER_DAC_0_PNP_APB_MASK                         0x00000100
#define GAISLER_DAC_0_PNP_APB_IRQ                          26
#define GAISLER_DAC_0_PNP_VERSION                          0

#define GAISLER_DAC_1_PNP_APB                              0x80409000
#define GAISLER_DAC_1_PNP_APB_MASK                         0x00000100
#define GAISLER_DAC_1_PNP_APB_IRQ                          27
#define GAISLER_DAC_1_PNP_VERSION                          0

#define GAISLER_DAC_2_PNP_APB                              0x8040a000
#define GAISLER_DAC_2_PNP_APB_MASK                         0x00000100
#define GAISLER_DAC_2_PNP_APB_IRQ                          36
#define GAISLER_DAC_2_PNP_VERSION                          0

#define GAISLER_DAC_3_PNP_APB                              0x8040b000
#define GAISLER_DAC_3_PNP_APB_MASK                         0x00000100
#define GAISLER_DAC_3_PNP_APB_IRQ                          37
#define GAISLER_DAC_3_PNP_VERSION                          0

#define GAISLER_FTMCTRL_0_PNP_AHB_0                        0x01000000
#define GAISLER_FTMCTRL_0_PNP_AHB_0_MASK                   0x01000000
#define GAISLER_FTMCTRL_0_PNP_AHB_2                        0x40000000
#define GAISLER_FTMCTRL_0_PNP_AHB_2_MASK                   0x10000000
#define GAISLER_FTMCTRL_0_PNP_APB                          0x80000000
#define GAISLER_FTMCTRL_0_PNP_APB_MASK                     0x00000100
#define GAISLER_FTMCTRL_0_PNP_VERSION                      2

#define GAISLER_FTMCTRL_1_PNP_AHB_0                        0x51000000
#define GAISLER_FTMCTRL_1_PNP_AHB_0_MASK                   0x01000000
#define GAISLER_FTMCTRL_1_PNP_AHB_2                        0x50000000
#define GAISLER_FTMCTRL_1_PNP_AHB_2_MASK                   0x01000000
#define GAISLER_FTMCTRL_1_PNP_APB                          0x80307000
#define GAISLER_FTMCTRL_1_PNP_APB_MASK                     0x00000100
#define GAISLER_FTMCTRL_1_PNP_VERSION                      2

#define GAISLER_GPIO_0_PNP_APB                             0x8030c000
#define GAISLER_GPIO_0_PNP_APB_MASK                        0x00001000
#define GAISLER_GPIO_0_PNP_APB_IRQ                         17
#define GAISLER_GPIO_0_PNP_VERSION                         3

#define GAISLER_GPIO_1_PNP_APB                             0x8030d000
#define GAISLER_GPIO_1_PNP_APB_MASK                        0x00001000
#define GAISLER_GPIO_1_PNP_APB_IRQ                         38
#define GAISLER_GPIO_1_PNP_VERSION                         3

#define GAISLER_GPREGBANK_0_PNP_APB                        0x8000d000
#define GAISLER_GPREGBANK_0_PNP_APB_MASK                   0x00000100
#define GAISLER_GPREGBANK_0_PNP_APB_IRQ                    63
#define GAISLER_GPREGBANK_0_PNP_VERSION                    0

#define GAISLER_GPREG_0_PNP_APB                            0x80008000
#define GAISLER_GPREG_0_PNP_APB_MASK                       0x00000100
#define GAISLER_GPREG_0_PNP_VERSION                        0

#define GAISLER_GPREG_1_PNP_APB                            0x8000e000
#define GAISLER_GPREG_1_PNP_APB_MASK                       0x00000100
#define GAISLER_GPREG_1_PNP_VERSION                        0

#define GAISLER_GPREG_2_PNP_APB                            0x94002000
#define GAISLER_GPREG_2_PNP_APB_MASK                       0x00000100
#define GAISLER_GPREG_2_PNP_VERSION                        0

#define GAISLER_GPTIMER_0_PNP_APB                          0x80003000
#define GAISLER_GPTIMER_0_PNP_APB_MASK                     0x00000100
#define GAISLER_GPTIMER_0_PNP_APB_IRQ                      9
#define GAISLER_GPTIMER_0_PNP_VERSION                      1

#define GAISLER_GPTIMER_1_PNP_APB                          0x80004000
#define GAISLER_GPTIMER_1_PNP_APB_MASK                     0x00000100
#define GAISLER_GPTIMER_1_PNP_APB_IRQ                      53
#define GAISLER_GPTIMER_1_PNP_VERSION                      1

#define GAISLER_GR1553B_0_PNP_APB                          0x80101000
#define GAISLER_GR1553B_0_PNP_APB_MASK                     0x00000100
#define GAISLER_GR1553B_0_PNP_APB_IRQ                      4
#define GAISLER_GR1553B_0_PNP_VERSION                      0

#define GAISLER_GRADCDAC_0_PNP_APB                         0x80308000
#define GAISLER_GRADCDAC_0_PNP_APB_MASK                    0x00000100
#define GAISLER_GRADCDAC_0_PNP_APB_IRQ                     16
#define GAISLER_GRADCDAC_0_PNP_VERSION                     0

#define GAISLER_GRCAN_0_PNP_APB                            0x80102000
#define GAISLER_GRCAN_0_PNP_APB_MASK                       0x00000400
#define GAISLER_GRCAN_0_PNP_APB_IRQ                        21
#define GAISLER_GRCAN_0_PNP_VERSION                        0

#define GAISLER_GRCAN_1_PNP_APB                            0x80103000
#define GAISLER_GRCAN_1_PNP_APB_MASK                       0x00000400
#define GAISLER_GRCAN_1_PNP_APB_IRQ                        21
#define GAISLER_GRCAN_1_PNP_VERSION                        0

#define GAISLER_GRDMAC_0_PNP_APB                           0x80106000
#define GAISLER_GRDMAC_0_PNP_APB_MASK                      0x00000200
#define GAISLER_GRDMAC_0_PNP_APB_IRQ                       6
#define GAISLER_GRDMAC_0_PNP_VERSION                       3

#define GAISLER_GRDMAC_1_PNP_APB                           0x80107000
#define GAISLER_GRDMAC_1_PNP_APB_MASK                      0x00000200
#define GAISLER_GRDMAC_1_PNP_APB_IRQ                       6
#define GAISLER_GRDMAC_1_PNP_VERSION                       3

#define GAISLER_GRDMAC_2_PNP_APB                           0x80108000
#define GAISLER_GRDMAC_2_PNP_APB_MASK                      0x00000200
#define GAISLER_GRDMAC_2_PNP_APB_IRQ                       6
#define GAISLER_GRDMAC_2_PNP_VERSION                       3

#define GAISLER_GRDMAC_3_PNP_APB                           0x80109000
#define GAISLER_GRDMAC_3_PNP_APB_MASK                      0x00000200
#define GAISLER_GRDMAC_3_PNP_APB_IRQ                       6
#define GAISLER_GRDMAC_3_PNP_VERSION                       3

#define GAISLER_GRMEMPROT_0_PNP_APB                        0x80005000
#define GAISLER_GRMEMPROT_0_PNP_APB_MASK                   0x00000200
#define GAISLER_GRMEMPROT_0_PNP_APB_IRQ                    63
#define GAISLER_GRMEMPROT_0_PNP_VERSION                    0

#define GAISLER_GRMEMPROT_1_PNP_APB                        0x8010a000
#define GAISLER_GRMEMPROT_1_PNP_APB_MASK                   0x00000100
#define GAISLER_GRMEMPROT_1_PNP_APB_IRQ                    63
#define GAISLER_GRMEMPROT_1_PNP_VERSION                    0

#define GAISLER_GRPLL_0_PNP_APB                            0x8010d000
#define GAISLER_GRPLL_0_PNP_APB_MASK                       0x00000100
#define GAISLER_GRPLL_0_PNP_APB_IRQ                        63
#define GAISLER_GRPLL_0_PNP_VERSION                        0

#define GAISLER_GRPWRX_0_PNP_APB                           0x8010e000
#define GAISLER_GRPWRX_0_PNP_APB_MASK                      0x00000100
#define GAISLER_GRPWRX_0_PNP_APB_IRQ                       2
#define GAISLER_GRPWRX_0_PNP_VERSION                       1

#define GAISLER_GRPWTX_0_PNP_APB                           0x8010f000
#define GAISLER_GRPWTX_0_PNP_APB_MASK                      0x00000100
#define GAISLER_GRPWTX_0_PNP_APB_IRQ                       3
#define GAISLER_GRPWTX_0_PNP_VERSION                       1

#define GAISLER_I2C2AHB_0_PNP_APB                          0x80105000
#define GAISLER_I2C2AHB_0_PNP_APB_MASK                     0x00000100
#define GAISLER_I2C2AHB_0_PNP_APB_IRQ                      47
#define GAISLER_I2C2AHB_0_PNP_VERSION                      0

#define GAISLER_I2CMST_0_PNP_APB                           0x8030e000
#define GAISLER_I2CMST_0_PNP_APB_MASK                      0x00000100
#define GAISLER_I2CMST_0_PNP_APB_IRQ                       50
#define GAISLER_I2CMST_0_PNP_VERSION                       3

#define GAISLER_I2CMST_1_PNP_APB                           0x8030f000
#define GAISLER_I2CMST_1_PNP_APB_MASK                      0x00000100
#define GAISLER_I2CMST_1_PNP_APB_IRQ                       51
#define GAISLER_I2CMST_1_PNP_VERSION                       3

#define GAISLER_I2CSLV_0_PNP_APB                           0x8040c000
#define GAISLER_I2CSLV_0_PNP_APB_MASK                      0x00000100
#define GAISLER_I2CSLV_0_PNP_APB_IRQ                       7
#define GAISLER_I2CSLV_0_PNP_VERSION                       0

#define GAISLER_I2CSLV_1_PNP_APB                           0x8040d000
#define GAISLER_I2CSLV_1_PNP_APB_MASK                      0x00000100
#define GAISLER_I2CSLV_1_PNP_APB_IRQ                       47
#define GAISLER_I2CSLV_1_PNP_VERSION                       0

#define GAISLER_IRQMP_0_PNP_APB                            0x80002000
#define GAISLER_IRQMP_0_PNP_APB_MASK                       0x00000400
#define GAISLER_IRQMP_0_PNP_VERSION                        4

#define GAISLER_L3STAT_0_PNP_APB                           0x80009000
#define GAISLER_L3STAT_0_PNP_APB_MASK                      0x00000400
#define GAISLER_L3STAT_0_PNP_VERSION                       1

#define GAISLER_L3STAT_1_PNP_APB                           0x94001000
#define GAISLER_L3STAT_1_PNP_APB_MASK                      0x00000400
#define GAISLER_L3STAT_1_PNP_VERSION                       1

#define GAISLER_LEON3DSU_0_PNP_AHB_0                       0x90000000
#define GAISLER_LEON3DSU_0_PNP_AHB_0_MASK                  0x04000000
#define GAISLER_LEON3DSU_0_PNP_VERSION                     2

#define GAISLER_LEON3FT_0_PNP_VERSION                      3

#define GAISLER_LRAM_0_PNP_AHB_0                           0x30000000
#define GAISLER_LRAM_0_PNP_AHB_0_MASK                      0x00100000
#define GAISLER_LRAM_0_PNP_APB                             0x80001000
#define GAISLER_LRAM_0_PNP_APB_MASK                        0x00000100
#define GAISLER_LRAM_0_PNP_APB_IRQ                         63
#define GAISLER_LRAM_0_PNP_VERSION                         16

#define GAISLER_LRAM_1_PNP_AHB_0                           0x31000000
#define GAISLER_LRAM_1_PNP_AHB_0_MASK                      0x00100000
#define GAISLER_LRAM_1_PNP_APB                             0x8000b000
#define GAISLER_LRAM_1_PNP_APB_MASK                        0x00000100
#define GAISLER_LRAM_1_PNP_APB_IRQ                         63
#define GAISLER_LRAM_1_PNP_VERSION                         17

#define GAISLER_MEMSCRUB_0_PNP_AHB_0                       0xfff00000
#define GAISLER_MEMSCRUB_0_PNP_AHB_0_MASK                  0x00000100
#define GAISLER_MEMSCRUB_0_PNP_AHB_IRQ                     63
#define GAISLER_MEMSCRUB_0_PNP_VERSION                     0

#define GAISLER_PWM_0_PNP_APB                              0x80310000
#define GAISLER_PWM_0_PNP_APB_MASK                         0x00000100
#define GAISLER_PWM_0_PNP_APB_IRQ                          8
#define GAISLER_PWM_0_PNP_VERSION                          2

#define GAISLER_PWM_1_PNP_APB                              0x80410000
#define GAISLER_PWM_1_PNP_APB_MASK                         0x00000100
#define GAISLER_PWM_1_PNP_APB_IRQ                          8
#define GAISLER_PWM_1_PNP_VERSION                          2

#define GAISLER_SPI2AHB_0_PNP_APB                          0x80104000
#define GAISLER_SPI2AHB_0_PNP_APB_MASK                     0x00000100
#define GAISLER_SPI2AHB_0_PNP_APB_IRQ                      62
#define GAISLER_SPI2AHB_0_PNP_VERSION                      0

#define GAISLER_SPICTRL_0_PNP_APB                          0x80309000
#define GAISLER_SPICTRL_0_PNP_APB_MASK                     0x00000100
#define GAISLER_SPICTRL_0_PNP_APB_IRQ                      48
#define GAISLER_SPICTRL_0_PNP_VERSION                      5

#define GAISLER_SPICTRL_1_PNP_APB                          0x8030a000
#define GAISLER_SPICTRL_1_PNP_APB_MASK                     0x00000100
#define GAISLER_SPICTRL_1_PNP_APB_IRQ                      49
#define GAISLER_SPICTRL_1_PNP_VERSION                      5

#define GAISLER_SPIMCTRL_0_PNP_AHB_0                       0xfff00100
#define GAISLER_SPIMCTRL_0_PNP_AHB_0_MASK                  0x00000100
#define GAISLER_SPIMCTRL_0_PNP_AHB_1                       0x02000000
#define GAISLER_SPIMCTRL_0_PNP_AHB_1_MASK                  0x02000000
#define GAISLER_SPIMCTRL_0_PNP_AHB_IRQ                     52
#define GAISLER_SPIMCTRL_0_PNP_VERSION                     1

#define GAISLER_SPIMCTRL_1_PNP_AHB_0                       0xfff00200
#define GAISLER_SPIMCTRL_1_PNP_AHB_0_MASK                  0x00000100
#define GAISLER_SPIMCTRL_1_PNP_AHB_1                       0x04000000
#define GAISLER_SPIMCTRL_1_PNP_AHB_1_MASK                  0x02000000
#define GAISLER_SPIMCTRL_1_PNP_AHB_IRQ                     52
#define GAISLER_SPIMCTRL_1_PNP_VERSION                     1

#define GAISLER_SPISLAVE_0_PNP_APB                         0x8040f000
#define GAISLER_SPISLAVE_0_PNP_APB_MASK                    0x00000100
#define GAISLER_SPISLAVE_0_PNP_APB_IRQ                     62
#define GAISLER_SPISLAVE_0_PNP_VERSION                     0

#define GAISLER_SPW2_0_PNP_MST_IRQ                         5
#define GAISLER_SPW2_0_PNP_APB                             0x80100000
#define GAISLER_SPW2_0_PNP_APB_MASK                        0x00000100
#define GAISLER_SPW2_0_PNP_APB_IRQ                         5
#define GAISLER_SPW2_0_PNP_VERSION                         2

#define GAISLER_SPWTDP_0_PNP_APB                           0x8000c000
#define GAISLER_SPWTDP_0_PNP_APB_MASK                      0x00000200
#define GAISLER_SPWTDP_0_PNP_APB_IRQ                       43
#define GAISLER_SPWTDP_0_PNP_VERSION                       1

#endif /* __BCC_BSP_PNP_H_ */

