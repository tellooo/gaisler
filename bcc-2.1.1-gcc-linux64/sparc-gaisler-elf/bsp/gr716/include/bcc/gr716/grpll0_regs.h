#ifndef GRPLL0_REGS_H
#define GRPLL0_REGS_H

/**
 * @file
 *
 * @brief Register description for grpll0
 *
 */

/* version: GRMON 3.0.15 */

#include <stdint.h>

/**
 * @brief GAISLER_UNKNOWN_0
 *
 * address: 0x8010d000
 *
 * Offset | Name       | Description
 * ------ | ---------- | --------------------------------
 * 0x0000 | cfg        | PLL configuration registers
 * 0x0004 | sts        | PLL status registers
 * 0x0008 | pllref     | PLL clock reference registers
 * 0x000c | spwref     | SpaceWire clock reference registers
 * 0x0010 | milref     | MIL-1553B clock reference registers
 * 0x0014 | sysref     | System clock reference registers
 * 0x0018 | syssel     | Select system clock source
 * 0x0020 | prot       | Clock protection registers
 * 0x0024 | tctrl      | Test clock configuration registers
 * 0x0028 | pwm0ref    | Select reference for PWM0 clock
 * 0x002c | pwm1ref    | Select reference for PWM1 clock
 */
struct grpll0_regs {
  /**
   * @brief PLL configuration registers
   *
   * offset: 0x0000
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31     | pd         | PLL power down
   *  2:0   | cfg        | PLL configuration
   */
  uint32_t cfg;

  /**
   * @brief PLL status registers
   *
   * offset: 0x0004
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  1     | ll         | PLL lost lock
   *  0     | cl         | PLL clock lock
   */
  uint32_t sts;

  /**
   * @brief PLL clock reference registers
   *
   * offset: 0x0008
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  9:8   | sel        | PLL Reference Clock
   */
  uint32_t pllref;

  /**
   * @brief SpaceWire clock reference registers
   *
   * offset: 0x000c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 23:16  | duty       | SpaceWire Reference Clock duty cycle
   *  9:8   | sel        | SpaceWire Reference Clock source
   *  7:0   | div        | SpaceWire Reference Clock Divisor
   */
  uint32_t spwref;

  /**
   * @brief MIL-1553B clock reference registers
   *
   * offset: 0x0010
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 23:16  | duty       | MIL-1553B Reference Clock duty cycle
   *  9:8   | sel        | MIL-1553B Reference Clock source
   *  7:0   | div        | MIL-1553B Reference Clock Divisor
   */
  uint32_t milref;

  /**
   * @brief System clock reference registers
   *
   * offset: 0x0014
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 23:16  | duty       | System Reference Clock duty cycle
   *  9:8   | sel        | System Reference Clock source
   *  7:0   | div        | System Reference Clock Divisor
   */
  uint32_t sysref;

  /**
   * @brief Select system clock source
   *
   * offset: 0x0018
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  0     | s          | Select new system clock source
   */
  uint32_t syssel;

  uint32_t _pad0x001c[0x0004 / 4];
  /**
   * @brief Clock protection registers
   *
   * offset: 0x0020
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 28:24  | ECTRL      | Enable error detection and correction
   * 21:20  | ESTAT      | Error status register
   * 17:16  | EIRQ       | Enable checksum error interrupt
   * 14:8   | REGE0      | EDAC checksum 0
   *  6:0   | REGE1      | EDAC checksum 1
   */
  uint32_t prot;

  /**
   * @brief Test clock configuration registers
   *
   * offset: 0x0024
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  0     | en         | Enable test output clocks
   */
  uint32_t tctrl;

  /**
   * @brief Select reference for PWM0 clock
   *
   * offset: 0x0028
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 24     | d2         | PWM0 Reference Clock divide by 2
   * 23:16  | duty       | PWM0 Reference Clock duty cycle
   *  9:8   | sel        | PWM0 Reference Clock source
   *  7:0   | div        | PWM0 Reference Clock Divisor
   */
  uint32_t pwm0ref;

  /**
   * @brief Select reference for PWM1 clock
   *
   * offset: 0x002c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 24     | d2         | PWM1 Reference Clock divide by 2
   * 23:16  | duty       | PWM1 Reference Clock duty cycle
   *  9:8   | sel        | PWM1 Reference Clock source
   *  7:0   | div        | PWM1 Reference Clock Divisor
   */
  uint32_t pwm1ref;

};

/** @brief PLL power down bit number*/
#define GRPLL0_CFG_PD_BIT                        31
/** @brief PLL configuration bit number*/
#define GRPLL0_CFG_CFG_BIT                        0

/** @brief PLL lost lock bit number*/
#define GRPLL0_STS_LL_BIT                         1
/** @brief PLL clock lock bit number*/
#define GRPLL0_STS_CL_BIT                         0

/** @brief PLL Reference Clock bit number*/
#define GRPLL0_PLLREF_SEL_BIT                     8

/** @brief SpaceWire Reference Clock duty cycle bit number*/
#define GRPLL0_SPWREF_DUTY_BIT                   16
/** @brief SpaceWire Reference Clock source bit number*/
#define GRPLL0_SPWREF_SEL_BIT                     8
/** @brief SpaceWire Reference Clock Divisor bit number*/
#define GRPLL0_SPWREF_DIV_BIT                     0

/** @brief MIL-1553B Reference Clock duty cycle bit number*/
#define GRPLL0_MILREF_DUTY_BIT                   16
/** @brief MIL-1553B Reference Clock source bit number*/
#define GRPLL0_MILREF_SEL_BIT                     8
/** @brief MIL-1553B Reference Clock Divisor bit number*/
#define GRPLL0_MILREF_DIV_BIT                     0

/** @brief System Reference Clock duty cycle bit number*/
#define GRPLL0_SYSREF_DUTY_BIT                   16
/** @brief System Reference Clock source bit number*/
#define GRPLL0_SYSREF_SEL_BIT                     8
/** @brief System Reference Clock Divisor bit number*/
#define GRPLL0_SYSREF_DIV_BIT                     0

/** @brief Select new system clock source bit number*/
#define GRPLL0_SYSSEL_S_BIT                       0

/** @brief Enable error detection and correction bit number*/
#define GRPLL0_PROT_ECTRL_BIT                    24
/** @brief Error status register bit number*/
#define GRPLL0_PROT_ESTAT_BIT                    20
/** @brief Enable checksum error interrupt bit number*/
#define GRPLL0_PROT_EIRQ_BIT                     16
/** @brief EDAC checksum 0 bit number*/
#define GRPLL0_PROT_REGE0_BIT                     8
/** @brief EDAC checksum 1 bit number*/
#define GRPLL0_PROT_REGE1_BIT                     0

/** @brief Enable test output clocks bit number*/
#define GRPLL0_TCTRL_EN_BIT                       0

/** @brief PWM0 Reference Clock divide by 2 bit number*/
#define GRPLL0_PWM0REF_D2_BIT                    24
/** @brief PWM0 Reference Clock duty cycle bit number*/
#define GRPLL0_PWM0REF_DUTY_BIT                  16
/** @brief PWM0 Reference Clock source bit number*/
#define GRPLL0_PWM0REF_SEL_BIT                    8
/** @brief PWM0 Reference Clock Divisor bit number*/
#define GRPLL0_PWM0REF_DIV_BIT                    0

/** @brief PWM1 Reference Clock divide by 2 bit number*/
#define GRPLL0_PWM1REF_D2_BIT                    24
/** @brief PWM1 Reference Clock duty cycle bit number*/
#define GRPLL0_PWM1REF_DUTY_BIT                  16
/** @brief PWM1 Reference Clock source bit number*/
#define GRPLL0_PWM1REF_SEL_BIT                    8
/** @brief PWM1 Reference Clock Divisor bit number*/
#define GRPLL0_PWM1REF_DIV_BIT                    0

/** @brief PLL power down mask */
#define GRPLL0_CFG_PD                            0x80000000
/** @brief PLL configuration mask */
#define GRPLL0_CFG_CFG                           0x00000007

/** @brief PLL lost lock mask */
#define GRPLL0_STS_LL                            0x00000002
/** @brief PLL clock lock mask */
#define GRPLL0_STS_CL                            0x00000001

/** @brief PLL Reference Clock mask */
#define GRPLL0_PLLREF_SEL                        0x00000300

/** @brief SpaceWire Reference Clock duty cycle mask */
#define GRPLL0_SPWREF_DUTY                       0x00ff0000
/** @brief SpaceWire Reference Clock source mask */
#define GRPLL0_SPWREF_SEL                        0x00000300
/** @brief SpaceWire Reference Clock Divisor mask */
#define GRPLL0_SPWREF_DIV                        0x000000ff

/** @brief MIL-1553B Reference Clock duty cycle mask */
#define GRPLL0_MILREF_DUTY                       0x00ff0000
/** @brief MIL-1553B Reference Clock source mask */
#define GRPLL0_MILREF_SEL                        0x00000300
/** @brief MIL-1553B Reference Clock Divisor mask */
#define GRPLL0_MILREF_DIV                        0x000000ff

/** @brief System Reference Clock duty cycle mask */
#define GRPLL0_SYSREF_DUTY                       0x00ff0000
/** @brief System Reference Clock source mask */
#define GRPLL0_SYSREF_SEL                        0x00000300
/** @brief System Reference Clock Divisor mask */
#define GRPLL0_SYSREF_DIV                        0x000000ff

/** @brief Select new system clock source mask */
#define GRPLL0_SYSSEL_S                          0x00000001

/** @brief Enable error detection and correction mask */
#define GRPLL0_PROT_ECTRL                        0x1f000000
/** @brief Error status register mask */
#define GRPLL0_PROT_ESTAT                        0x00300000
/** @brief Enable checksum error interrupt mask */
#define GRPLL0_PROT_EIRQ                         0x00030000
/** @brief EDAC checksum 0 mask */
#define GRPLL0_PROT_REGE0                        0x00007f00
/** @brief EDAC checksum 1 mask */
#define GRPLL0_PROT_REGE1                        0x0000007f

/** @brief Enable test output clocks mask */
#define GRPLL0_TCTRL_EN                          0x00000001

/** @brief PWM0 Reference Clock divide by 2 mask */
#define GRPLL0_PWM0REF_D2                        0x01000000
/** @brief PWM0 Reference Clock duty cycle mask */
#define GRPLL0_PWM0REF_DUTY                      0x00ff0000
/** @brief PWM0 Reference Clock source mask */
#define GRPLL0_PWM0REF_SEL                       0x00000300
/** @brief PWM0 Reference Clock Divisor mask */
#define GRPLL0_PWM0REF_DIV                       0x000000ff

/** @brief PWM1 Reference Clock divide by 2 mask */
#define GRPLL0_PWM1REF_D2                        0x01000000
/** @brief PWM1 Reference Clock duty cycle mask */
#define GRPLL0_PWM1REF_DUTY                      0x00ff0000
/** @brief PWM1 Reference Clock source mask */
#define GRPLL0_PWM1REF_SEL                       0x00000300
/** @brief PWM1 Reference Clock Divisor mask */
#define GRPLL0_PWM1REF_DIV                       0x000000ff


#endif

