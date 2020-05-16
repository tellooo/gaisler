#ifndef GPREG1_REGS_H
#define GPREG1_REGS_H

/**
 * @file
 *
 * @brief Register description for gpreg1
 *
 */

/* version: GRMON 3.0.15 */

#include <stdint.h>

/**
 * @brief GAISLER_GPREGBANK_0
 *
 * address: 0x8000d000
 *
 * Offset | Name       | Description
 * ------ | ---------- | --------------------------------
 * 0x0000 | gpio0      | System IO register for GPIO 0 to 7
 * 0x0004 | gpio1      | System IO register for GPIO 8 to 15
 * 0x0008 | gpio2      | System IO register for GPIO 16 to 23
 * 0x000c | gpio3      | System IO register for GPIO 24 to 31
 * 0x0010 | gpio4      | System IO register for GPIO 32 to 39
 * 0x0014 | gpio5      | System IO register for GPIO 40 to 47
 * 0x0018 | gpio6      | System IO register for GPIO 48 to 55
 * 0x001c | gpio7      | System IO register for GPIO 56 to 63
 * 0x0020 | pullup0    | Pullup register for GPIO 0 to 31
 * 0x0024 | pullup1    | Pullup register for GPIO 32 to 63
 * 0x0028 | pulldown0  | Pulldown register for GPIO 0 to 31
 * 0x002c | pulldown1  | Pulldown register for GPIO 32 to 63
 * 0x0030 | lvds       | IO configuration for LVDS
 * 0x0034 | prot       | Configuration protection register
 * 0x0038 | eirq       | Config register error interrupt
 * 0x003c | estat      | Config register error status
 */
struct gpreg1_regs {
  /**
   * @brief System IO register for GPIO 0 to 7
   *
   * offset: 0x0000
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio7      | Functional select for GPIO 7
   * 27:24  | gpio6      | Functional select for GPIO 6
   * 23:20  | gpio5      | Functional select for GPIO 5
   * 19:16  | gpio4      | Functional select for GPIO 4
   * 15:12  | gpio3      | Functional select for GPIO 3
   * 11:8   | gpio2      | Functional select for GPIO 2
   *  7:4   | gpio1      | Functional select for GPIO 1
   *  3:0   | gpio0      | Functional select for GPIO 0
   */
  uint32_t gpio0;

  /**
   * @brief System IO register for GPIO 8 to 15
   *
   * offset: 0x0004
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio15     | Functional select for GPIO 15
   * 27:24  | gpio14     | Functional select for GPIO 14
   * 23:20  | gpio13     | Functional select for GPIO 13
   * 19:16  | gpio12     | Functional select for GPIO 12
   * 15:12  | gpio11     | Functional select for GPIO 11
   * 11:8   | gpio10     | Functional select for GPIO 10
   *  7:4   | gpio9      | Functional select for GPIO 9
   *  3:0   | gpio8      | Functional select for GPIO 8
   */
  uint32_t gpio1;

  /**
   * @brief System IO register for GPIO 16 to 23
   *
   * offset: 0x0008
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio23     | Functional select for GPIO 23
   * 27:24  | gpio22     | Functional select for GPIO 22
   * 23:20  | gpio21     | Functional select for GPIO 21
   * 19:16  | gpio20     | Functional select for GPIO 20
   * 15:12  | gpio19     | Functional select for GPIO 19
   * 11:8   | gpio18     | Functional select for GPIO 18
   *  7:4   | gpio17     | Functional select for GPIO 17
   *  3:0   | gpio16     | Functional select for GPIO 16
   */
  uint32_t gpio2;

  /**
   * @brief System IO register for GPIO 24 to 31
   *
   * offset: 0x000c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio31     | Functional select for GPIO 31
   * 27:24  | gpio30     | Functional select for GPIO 30
   * 23:20  | gpio29     | Functional select for GPIO 29
   * 19:16  | gpio28     | Functional select for GPIO 28
   * 15:12  | gpio27     | Functional select for GPIO 27
   * 11:8   | gpio26     | Functional select for GPIO 26
   *  7:4   | gpio25     | Functional select for GPIO 25
   *  3:0   | gpio24     | Functional select for GPIO 24
   */
  uint32_t gpio3;

  /**
   * @brief System IO register for GPIO 32 to 39
   *
   * offset: 0x0010
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio39     | Functional select for GPIO 39
   * 27:24  | gpio38     | Functional select for GPIO 38
   * 23:20  | gpio37     | Functional select for GPIO 37
   * 19:16  | gpio36     | Functional select for GPIO 36
   * 15:12  | gpio35     | Functional select for GPIO 35
   * 11:8   | gpio34     | Functional select for GPIO 34
   *  7:4   | gpio33     | Functional select for GPIO 33
   *  3:0   | gpio32     | Functional select for GPIO 32
   */
  uint32_t gpio4;

  /**
   * @brief System IO register for GPIO 40 to 47
   *
   * offset: 0x0014
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio47     | Functional select for GPIO 47
   * 27:24  | gpio46     | Functional select for GPIO 46
   * 23:20  | gpio45     | Functional select for GPIO 45
   * 19:16  | gpio44     | Functional select for GPIO 44
   * 15:12  | gpio43     | Functional select for GPIO 43
   * 11:8   | gpio42     | Functional select for GPIO 42
   *  7:4   | gpio41     | Functional select for GPIO 41
   *  3:0   | gpio40     | Functional select for GPIO 40
   */
  uint32_t gpio5;

  /**
   * @brief System IO register for GPIO 48 to 55
   *
   * offset: 0x0018
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio55     | Functional select for GPIO 55
   * 27:24  | gpio54     | Functional select for GPIO 54
   * 23:20  | gpio53     | Functional select for GPIO 53
   * 19:16  | gpio52     | Functional select for GPIO 52
   * 15:12  | gpio51     | Functional select for GPIO 51
   * 11:8   | gpio50     | Functional select for GPIO 50
   *  7:4   | gpio49     | Functional select for GPIO 49
   *  3:0   | gpio48     | Functional select for GPIO 48
   */
  uint32_t gpio6;

  /**
   * @brief System IO register for GPIO 56 to 63
   *
   * offset: 0x001c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:28  | gpio63     | Functional select for GPIO 63
   * 27:24  | gpio62     | Functional select for GPIO 62
   * 23:20  | gpio61     | Functional select for GPIO 61
   * 19:16  | gpio60     | Functional select for GPIO 60
   * 15:12  | gpio59     | Functional select for GPIO 59
   * 11:8   | gpio58     | Functional select for GPIO 58
   *  7:4   | gpio57     | Functional select for GPIO 57
   *  3:0   | gpio56     | Functional select for GPIO 56
   */
  uint32_t gpio7;

  /**
   * @brief Pullup register for GPIO 0 to 31
   *
   * offset: 0x0020
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:0   | up         | Pullup config for GPIO 0 to 31
   */
  uint32_t pullup0;

  /**
   * @brief Pullup register for GPIO 32 to 63
   *
   * offset: 0x0024
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:0   | up         | Pullup config for GPIO 32 to 63
   */
  uint32_t pullup1;

  /**
   * @brief Pulldown register for GPIO 0 to 31
   *
   * offset: 0x0028
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:0   | down       | Pulldown config for GPIO 0 to 31
   */
  uint32_t pulldown0;

  /**
   * @brief Pulldown register for GPIO 32 to 63
   *
   * offset: 0x002c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 31:0   | down       | Pulldown config for GPIO 32 to 63
   */
  uint32_t pulldown1;

  /**
   * @brief IO configuration for LVDS
   *
   * offset: 0x0030
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 15:12  | rx0        | Select functionality for LVDS RX 0
   * 11:8   | tx2        | Select functionality for LVDS TX 2
   *  7:4   | tx1        | Select functionality for LVDS TX 1
   *  3:0   | tx0        | Select functionality for LVDS TX 0
   */
  uint32_t lvds;

  /**
   * @brief Configuration protection register
   *
   * offset: 0x0034
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  3     | di         | Disable IO at multiple errors
   *  2     | ec         | Enable correction of single error
   *  1     | ir         | Generate interrupt at error event
   *  0     | en         | Enable System IO Register Protection
   */
  uint32_t prot;

  /**
   * @brief Config register error interrupt
   *
   * offset: 0x0038
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   *  1     | m          | Multiple error interrupt
   *  0     | e          | Single Error interrupt
   */
  uint32_t eirq;

  /**
   * @brief Config register error status
   *
   * offset: 0x003c
   *
   * Bit    | Name       | Description
   * ------ | ---------- | --------------------------------
   * 11:0   | estat      | Error status
   */
  uint32_t estat;

};

/** @brief Functional select for GPIO 7 bit number*/
#define GPREG1_GPIO0_GPIO7_BIT                   28
/** @brief Functional select for GPIO 6 bit number*/
#define GPREG1_GPIO0_GPIO6_BIT                   24
/** @brief Functional select for GPIO 5 bit number*/
#define GPREG1_GPIO0_GPIO5_BIT                   20
/** @brief Functional select for GPIO 4 bit number*/
#define GPREG1_GPIO0_GPIO4_BIT                   16
/** @brief Functional select for GPIO 3 bit number*/
#define GPREG1_GPIO0_GPIO3_BIT                   12
/** @brief Functional select for GPIO 2 bit number*/
#define GPREG1_GPIO0_GPIO2_BIT                    8
/** @brief Functional select for GPIO 1 bit number*/
#define GPREG1_GPIO0_GPIO1_BIT                    4
/** @brief Functional select for GPIO 0 bit number*/
#define GPREG1_GPIO0_GPIO0_BIT                    0

/** @brief Functional select for GPIO 15 bit number*/
#define GPREG1_GPIO1_GPIO15_BIT                  28
/** @brief Functional select for GPIO 14 bit number*/
#define GPREG1_GPIO1_GPIO14_BIT                  24
/** @brief Functional select for GPIO 13 bit number*/
#define GPREG1_GPIO1_GPIO13_BIT                  20
/** @brief Functional select for GPIO 12 bit number*/
#define GPREG1_GPIO1_GPIO12_BIT                  16
/** @brief Functional select for GPIO 11 bit number*/
#define GPREG1_GPIO1_GPIO11_BIT                  12
/** @brief Functional select for GPIO 10 bit number*/
#define GPREG1_GPIO1_GPIO10_BIT                   8
/** @brief Functional select for GPIO 9 bit number*/
#define GPREG1_GPIO1_GPIO9_BIT                    4
/** @brief Functional select for GPIO 8 bit number*/
#define GPREG1_GPIO1_GPIO8_BIT                    0

/** @brief Functional select for GPIO 23 bit number*/
#define GPREG1_GPIO2_GPIO23_BIT                  28
/** @brief Functional select for GPIO 22 bit number*/
#define GPREG1_GPIO2_GPIO22_BIT                  24
/** @brief Functional select for GPIO 21 bit number*/
#define GPREG1_GPIO2_GPIO21_BIT                  20
/** @brief Functional select for GPIO 20 bit number*/
#define GPREG1_GPIO2_GPIO20_BIT                  16
/** @brief Functional select for GPIO 19 bit number*/
#define GPREG1_GPIO2_GPIO19_BIT                  12
/** @brief Functional select for GPIO 18 bit number*/
#define GPREG1_GPIO2_GPIO18_BIT                   8
/** @brief Functional select for GPIO 17 bit number*/
#define GPREG1_GPIO2_GPIO17_BIT                   4
/** @brief Functional select for GPIO 16 bit number*/
#define GPREG1_GPIO2_GPIO16_BIT                   0

/** @brief Functional select for GPIO 31 bit number*/
#define GPREG1_GPIO3_GPIO31_BIT                  28
/** @brief Functional select for GPIO 30 bit number*/
#define GPREG1_GPIO3_GPIO30_BIT                  24
/** @brief Functional select for GPIO 29 bit number*/
#define GPREG1_GPIO3_GPIO29_BIT                  20
/** @brief Functional select for GPIO 28 bit number*/
#define GPREG1_GPIO3_GPIO28_BIT                  16
/** @brief Functional select for GPIO 27 bit number*/
#define GPREG1_GPIO3_GPIO27_BIT                  12
/** @brief Functional select for GPIO 26 bit number*/
#define GPREG1_GPIO3_GPIO26_BIT                   8
/** @brief Functional select for GPIO 25 bit number*/
#define GPREG1_GPIO3_GPIO25_BIT                   4
/** @brief Functional select for GPIO 24 bit number*/
#define GPREG1_GPIO3_GPIO24_BIT                   0

/** @brief Functional select for GPIO 39 bit number*/
#define GPREG1_GPIO4_GPIO39_BIT                  28
/** @brief Functional select for GPIO 38 bit number*/
#define GPREG1_GPIO4_GPIO38_BIT                  24
/** @brief Functional select for GPIO 37 bit number*/
#define GPREG1_GPIO4_GPIO37_BIT                  20
/** @brief Functional select for GPIO 36 bit number*/
#define GPREG1_GPIO4_GPIO36_BIT                  16
/** @brief Functional select for GPIO 35 bit number*/
#define GPREG1_GPIO4_GPIO35_BIT                  12
/** @brief Functional select for GPIO 34 bit number*/
#define GPREG1_GPIO4_GPIO34_BIT                   8
/** @brief Functional select for GPIO 33 bit number*/
#define GPREG1_GPIO4_GPIO33_BIT                   4
/** @brief Functional select for GPIO 32 bit number*/
#define GPREG1_GPIO4_GPIO32_BIT                   0

/** @brief Functional select for GPIO 47 bit number*/
#define GPREG1_GPIO5_GPIO47_BIT                  28
/** @brief Functional select for GPIO 46 bit number*/
#define GPREG1_GPIO5_GPIO46_BIT                  24
/** @brief Functional select for GPIO 45 bit number*/
#define GPREG1_GPIO5_GPIO45_BIT                  20
/** @brief Functional select for GPIO 44 bit number*/
#define GPREG1_GPIO5_GPIO44_BIT                  16
/** @brief Functional select for GPIO 43 bit number*/
#define GPREG1_GPIO5_GPIO43_BIT                  12
/** @brief Functional select for GPIO 42 bit number*/
#define GPREG1_GPIO5_GPIO42_BIT                   8
/** @brief Functional select for GPIO 41 bit number*/
#define GPREG1_GPIO5_GPIO41_BIT                   4
/** @brief Functional select for GPIO 40 bit number*/
#define GPREG1_GPIO5_GPIO40_BIT                   0

/** @brief Functional select for GPIO 55 bit number*/
#define GPREG1_GPIO6_GPIO55_BIT                  28
/** @brief Functional select for GPIO 54 bit number*/
#define GPREG1_GPIO6_GPIO54_BIT                  24
/** @brief Functional select for GPIO 53 bit number*/
#define GPREG1_GPIO6_GPIO53_BIT                  20
/** @brief Functional select for GPIO 52 bit number*/
#define GPREG1_GPIO6_GPIO52_BIT                  16
/** @brief Functional select for GPIO 51 bit number*/
#define GPREG1_GPIO6_GPIO51_BIT                  12
/** @brief Functional select for GPIO 50 bit number*/
#define GPREG1_GPIO6_GPIO50_BIT                   8
/** @brief Functional select for GPIO 49 bit number*/
#define GPREG1_GPIO6_GPIO49_BIT                   4
/** @brief Functional select for GPIO 48 bit number*/
#define GPREG1_GPIO6_GPIO48_BIT                   0

/** @brief Functional select for GPIO 63 bit number*/
#define GPREG1_GPIO7_GPIO63_BIT                  28
/** @brief Functional select for GPIO 62 bit number*/
#define GPREG1_GPIO7_GPIO62_BIT                  24
/** @brief Functional select for GPIO 61 bit number*/
#define GPREG1_GPIO7_GPIO61_BIT                  20
/** @brief Functional select for GPIO 60 bit number*/
#define GPREG1_GPIO7_GPIO60_BIT                  16
/** @brief Functional select for GPIO 59 bit number*/
#define GPREG1_GPIO7_GPIO59_BIT                  12
/** @brief Functional select for GPIO 58 bit number*/
#define GPREG1_GPIO7_GPIO58_BIT                   8
/** @brief Functional select for GPIO 57 bit number*/
#define GPREG1_GPIO7_GPIO57_BIT                   4
/** @brief Functional select for GPIO 56 bit number*/
#define GPREG1_GPIO7_GPIO56_BIT                   0

/** @brief Pullup config for GPIO 0 to 31 bit number*/
#define GPREG1_PULLUP0_UP_BIT                     0

/** @brief Pullup config for GPIO 32 to 63 bit number*/
#define GPREG1_PULLUP1_UP_BIT                     0

/** @brief Pulldown config for GPIO 0 to 31 bit number*/
#define GPREG1_PULLDOWN0_DOWN_BIT                 0

/** @brief Pulldown config for GPIO 32 to 63 bit number*/
#define GPREG1_PULLDOWN1_DOWN_BIT                 0

/** @brief Select functionality for LVDS RX 0 bit number*/
#define GPREG1_LVDS_RX0_BIT                      12
/** @brief Select functionality for LVDS TX 2 bit number*/
#define GPREG1_LVDS_TX2_BIT                       8
/** @brief Select functionality for LVDS TX 1 bit number*/
#define GPREG1_LVDS_TX1_BIT                       4
/** @brief Select functionality for LVDS TX 0 bit number*/
#define GPREG1_LVDS_TX0_BIT                       0

/** @brief Disable IO at multiple errors bit number*/
#define GPREG1_PROT_DI_BIT                        3
/** @brief Enable correction of single error bit number*/
#define GPREG1_PROT_EC_BIT                        2
/** @brief Generate interrupt at error event bit number*/
#define GPREG1_PROT_IR_BIT                        1
/** @brief Enable System IO Register Protection bit number*/
#define GPREG1_PROT_EN_BIT                        0

/** @brief Multiple error interrupt bit number*/
#define GPREG1_EIRQ_M_BIT                         1
/** @brief Single Error interrupt bit number*/
#define GPREG1_EIRQ_E_BIT                         0

/** @brief Error status bit number*/
#define GPREG1_ESTAT_ESTAT_BIT                    0

/** @brief Functional select for GPIO 7 mask */
#define GPREG1_GPIO0_GPIO7                       0xf0000000
/** @brief Functional select for GPIO 6 mask */
#define GPREG1_GPIO0_GPIO6                       0x0f000000
/** @brief Functional select for GPIO 5 mask */
#define GPREG1_GPIO0_GPIO5                       0x00f00000
/** @brief Functional select for GPIO 4 mask */
#define GPREG1_GPIO0_GPIO4                       0x000f0000
/** @brief Functional select for GPIO 3 mask */
#define GPREG1_GPIO0_GPIO3                       0x0000f000
/** @brief Functional select for GPIO 2 mask */
#define GPREG1_GPIO0_GPIO2                       0x00000f00
/** @brief Functional select for GPIO 1 mask */
#define GPREG1_GPIO0_GPIO1                       0x000000f0
/** @brief Functional select for GPIO 0 mask */
#define GPREG1_GPIO0_GPIO0                       0x0000000f

/** @brief Functional select for GPIO 15 mask */
#define GPREG1_GPIO1_GPIO15                      0xf0000000
/** @brief Functional select for GPIO 14 mask */
#define GPREG1_GPIO1_GPIO14                      0x0f000000
/** @brief Functional select for GPIO 13 mask */
#define GPREG1_GPIO1_GPIO13                      0x00f00000
/** @brief Functional select for GPIO 12 mask */
#define GPREG1_GPIO1_GPIO12                      0x000f0000
/** @brief Functional select for GPIO 11 mask */
#define GPREG1_GPIO1_GPIO11                      0x0000f000
/** @brief Functional select for GPIO 10 mask */
#define GPREG1_GPIO1_GPIO10                      0x00000f00
/** @brief Functional select for GPIO 9 mask */
#define GPREG1_GPIO1_GPIO9                       0x000000f0
/** @brief Functional select for GPIO 8 mask */
#define GPREG1_GPIO1_GPIO8                       0x0000000f

/** @brief Functional select for GPIO 23 mask */
#define GPREG1_GPIO2_GPIO23                      0xf0000000
/** @brief Functional select for GPIO 22 mask */
#define GPREG1_GPIO2_GPIO22                      0x0f000000
/** @brief Functional select for GPIO 21 mask */
#define GPREG1_GPIO2_GPIO21                      0x00f00000
/** @brief Functional select for GPIO 20 mask */
#define GPREG1_GPIO2_GPIO20                      0x000f0000
/** @brief Functional select for GPIO 19 mask */
#define GPREG1_GPIO2_GPIO19                      0x0000f000
/** @brief Functional select for GPIO 18 mask */
#define GPREG1_GPIO2_GPIO18                      0x00000f00
/** @brief Functional select for GPIO 17 mask */
#define GPREG1_GPIO2_GPIO17                      0x000000f0
/** @brief Functional select for GPIO 16 mask */
#define GPREG1_GPIO2_GPIO16                      0x0000000f

/** @brief Functional select for GPIO 31 mask */
#define GPREG1_GPIO3_GPIO31                      0xf0000000
/** @brief Functional select for GPIO 30 mask */
#define GPREG1_GPIO3_GPIO30                      0x0f000000
/** @brief Functional select for GPIO 29 mask */
#define GPREG1_GPIO3_GPIO29                      0x00f00000
/** @brief Functional select for GPIO 28 mask */
#define GPREG1_GPIO3_GPIO28                      0x000f0000
/** @brief Functional select for GPIO 27 mask */
#define GPREG1_GPIO3_GPIO27                      0x0000f000
/** @brief Functional select for GPIO 26 mask */
#define GPREG1_GPIO3_GPIO26                      0x00000f00
/** @brief Functional select for GPIO 25 mask */
#define GPREG1_GPIO3_GPIO25                      0x000000f0
/** @brief Functional select for GPIO 24 mask */
#define GPREG1_GPIO3_GPIO24                      0x0000000f

/** @brief Functional select for GPIO 39 mask */
#define GPREG1_GPIO4_GPIO39                      0xf0000000
/** @brief Functional select for GPIO 38 mask */
#define GPREG1_GPIO4_GPIO38                      0x0f000000
/** @brief Functional select for GPIO 37 mask */
#define GPREG1_GPIO4_GPIO37                      0x00f00000
/** @brief Functional select for GPIO 36 mask */
#define GPREG1_GPIO4_GPIO36                      0x000f0000
/** @brief Functional select for GPIO 35 mask */
#define GPREG1_GPIO4_GPIO35                      0x0000f000
/** @brief Functional select for GPIO 34 mask */
#define GPREG1_GPIO4_GPIO34                      0x00000f00
/** @brief Functional select for GPIO 33 mask */
#define GPREG1_GPIO4_GPIO33                      0x000000f0
/** @brief Functional select for GPIO 32 mask */
#define GPREG1_GPIO4_GPIO32                      0x0000000f

/** @brief Functional select for GPIO 47 mask */
#define GPREG1_GPIO5_GPIO47                      0xf0000000
/** @brief Functional select for GPIO 46 mask */
#define GPREG1_GPIO5_GPIO46                      0x0f000000
/** @brief Functional select for GPIO 45 mask */
#define GPREG1_GPIO5_GPIO45                      0x00f00000
/** @brief Functional select for GPIO 44 mask */
#define GPREG1_GPIO5_GPIO44                      0x000f0000
/** @brief Functional select for GPIO 43 mask */
#define GPREG1_GPIO5_GPIO43                      0x0000f000
/** @brief Functional select for GPIO 42 mask */
#define GPREG1_GPIO5_GPIO42                      0x00000f00
/** @brief Functional select for GPIO 41 mask */
#define GPREG1_GPIO5_GPIO41                      0x000000f0
/** @brief Functional select for GPIO 40 mask */
#define GPREG1_GPIO5_GPIO40                      0x0000000f

/** @brief Functional select for GPIO 55 mask */
#define GPREG1_GPIO6_GPIO55                      0xf0000000
/** @brief Functional select for GPIO 54 mask */
#define GPREG1_GPIO6_GPIO54                      0x0f000000
/** @brief Functional select for GPIO 53 mask */
#define GPREG1_GPIO6_GPIO53                      0x00f00000
/** @brief Functional select for GPIO 52 mask */
#define GPREG1_GPIO6_GPIO52                      0x000f0000
/** @brief Functional select for GPIO 51 mask */
#define GPREG1_GPIO6_GPIO51                      0x0000f000
/** @brief Functional select for GPIO 50 mask */
#define GPREG1_GPIO6_GPIO50                      0x00000f00
/** @brief Functional select for GPIO 49 mask */
#define GPREG1_GPIO6_GPIO49                      0x000000f0
/** @brief Functional select for GPIO 48 mask */
#define GPREG1_GPIO6_GPIO48                      0x0000000f

/** @brief Functional select for GPIO 63 mask */
#define GPREG1_GPIO7_GPIO63                      0xf0000000
/** @brief Functional select for GPIO 62 mask */
#define GPREG1_GPIO7_GPIO62                      0x0f000000
/** @brief Functional select for GPIO 61 mask */
#define GPREG1_GPIO7_GPIO61                      0x00f00000
/** @brief Functional select for GPIO 60 mask */
#define GPREG1_GPIO7_GPIO60                      0x000f0000
/** @brief Functional select for GPIO 59 mask */
#define GPREG1_GPIO7_GPIO59                      0x0000f000
/** @brief Functional select for GPIO 58 mask */
#define GPREG1_GPIO7_GPIO58                      0x00000f00
/** @brief Functional select for GPIO 57 mask */
#define GPREG1_GPIO7_GPIO57                      0x000000f0
/** @brief Functional select for GPIO 56 mask */
#define GPREG1_GPIO7_GPIO56                      0x0000000f

/** @brief Pullup config for GPIO 0 to 31 mask */
#define GPREG1_PULLUP0_UP                        0xffffffff

/** @brief Pullup config for GPIO 32 to 63 mask */
#define GPREG1_PULLUP1_UP                        0xffffffff

/** @brief Pulldown config for GPIO 0 to 31 mask */
#define GPREG1_PULLDOWN0_DOWN                    0xffffffff

/** @brief Pulldown config for GPIO 32 to 63 mask */
#define GPREG1_PULLDOWN1_DOWN                    0xffffffff

/** @brief Select functionality for LVDS RX 0 mask */
#define GPREG1_LVDS_RX0                          0x0000f000
/** @brief Select functionality for LVDS TX 2 mask */
#define GPREG1_LVDS_TX2                          0x00000f00
/** @brief Select functionality for LVDS TX 1 mask */
#define GPREG1_LVDS_TX1                          0x000000f0
/** @brief Select functionality for LVDS TX 0 mask */
#define GPREG1_LVDS_TX0                          0x0000000f

/** @brief Disable IO at multiple errors mask */
#define GPREG1_PROT_DI                           0x00000008
/** @brief Enable correction of single error mask */
#define GPREG1_PROT_EC                           0x00000004
/** @brief Generate interrupt at error event mask */
#define GPREG1_PROT_IR                           0x00000002
/** @brief Enable System IO Register Protection mask */
#define GPREG1_PROT_EN                           0x00000001

/** @brief Multiple error interrupt mask */
#define GPREG1_EIRQ_M                            0x00000002
/** @brief Single Error interrupt mask */
#define GPREG1_EIRQ_E                            0x00000001

/** @brief Error status mask */
#define GPREG1_ESTAT_ESTAT                       0x00000fff


#endif

