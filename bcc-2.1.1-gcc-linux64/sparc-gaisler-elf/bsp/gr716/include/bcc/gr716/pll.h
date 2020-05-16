#ifndef GR716_PLL_H
#define GR716_PLL_H

#define CLK_SOURCE_CLK   0x0
#define CLK_SOURCE_SPW   0x1
#define CLK_SOURCE_PLL   0x2

/*
 * System clock configuration
 *
 * source : Select source
 * div    : Div
 * duty   : Duty cycle
 * return : 0 on success, else non-zero
 *
 * Description:
 *
 * Mode #1: (Source = 0, Div = 0 and Duty = 0)
 *    - This mode bypasses the divider and uses the external clock pin as input
 * 
 * Mode #2: (Source = X, Div = Y and Duty = 0)
 *   - This mode divides the clock source with 2^Y and selects as system clock
 *   - Y can be in the range from 1 to 31
 * 
 * Mode #3: (Source = X, Div = Y and Duty = Z)
 *    - This mode divides the clock source with Y and selects as system clock.
 *    - Clock duty cycle is set by 'Duty' in number of system clocks
 *    - Y can be in the range from 2 to 31 and must be at least 1 greater than X
 *    - Z can be in the range from 1 to 30
 * 
 */
int gr716_sysclk(
        int source,
        int div,
        int duty
);

/*
 * SpaceWire clock configuration
 *
 * source : Select source
 * div    : Div
 * duty   : Duty cycle
 * return : 0 on success, else non-zero
 * 
 * Description:
 *
 * Mode #1: (Source = 0, Div = 0 and Duty = 0)
 *    - This mode bypasses the divider and uses the external clock pin as input 
 * 
 * Mode #2: (Source = X, Div = Y and Duty = 0)
 *   - This mode divides the clock source with 2^Y and selects as system clock
 *   - Y can be in the range from 1 to 31
 * 
 * Mode #3: (Source = X, Div = Y and Duty = Z)
 *    - This mode divides the clock source with Y and selects as system clock.
 *    - Clock duty cycle is set by 'Duty' in number of system clocks
 *    - Y can be in the range from 2 to 31 and must be at least 1 greater than X
 *    - Z can be in the range from 1 to 30 
 * 
 */

#define SPWCLK_SOURCE_CLK   0x0 
#define SPWCLK_SOURCE_PLL   0x1

int gr716_spwclk(
        int source,
        int div,
        int duty
);

/*
 * PLL configuration
 *
 * ref    : Select source
 * cfg    : PLL input source
 * pd     : PLL Power down
 * return : 0 on success, else non-zero
 * 
 * Description:
 * 
 *  Mode #1: (PD= 1, ref = X and cfg = Y)
 *   - This mode power down the PLL
 *
 *  Mode #2: (PD= 0, ref = X and cfg = Y)
 *   - This mode power on the PLL
 *   - Select the external input pin with 'ref'
 *   - The PLL must be configure the correct input frequency with 'cfg'
 */

#define PLL_POWER_DOWN    0x1 
#define PLL_POWER_ENABLE  0x0 

#define PLL_FREQ_50MHZ    0x0
#define PLL_FREQ_25MHZ    0x1
#define PLL_FREQ_20MHZ    0x2
#define PLL_FREQ_12MHZ    0x3
#define PLL_FREQ_10MHZ    0x4
#define PLL_FREQ_5MHZ     0x5

#define PLL_REF_SYS_CLK   0x0 
#define PLL_REF_SPW_CLK   0x1 

int gr716_pll_config(
        int ref,
        int cfg,
        int pd
);

/*
 * Determine if PLL is currently locked
 *
 * This function returns the current value of the PLL lock output.
 *
 * return: 1 if locked, else 0
 */
int gr716_pll_islocked(void);

#endif

