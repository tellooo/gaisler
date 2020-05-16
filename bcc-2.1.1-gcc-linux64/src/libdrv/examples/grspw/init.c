#include <bcc/capability.h>

#if defined(BCC_BSP_gr716)

#include <bcc/bcc_param.h>
#include <bcc/gr716/pll.h>
#include <bcc/gr716/pin.h>
#include <drv/clkgate.h>
#include <drv/gr716/clkgate.h>
#include <drv/regs/clkgate_bits.h>

static void enable_the_grspw2_clock(void)
{
        const uint32_t GATEMASK = CLKGATE1_GR716_GRSPW;
        struct clkgate_priv *dev;

        if (0 == clkgate_dev_count()) {
                clkgate_init(GR716_CLKGATE_DRV_ALL);
        }
        dev = clkgate_open(1);
        clkgate_gate(dev, GATEMASK);
        clkgate_enable(dev, GATEMASK);
        clkgate_close(dev);
}

static void enable_the_grspw2_pins(void)
{
        gr716_set_pinfunc(21, IO_MODE_SPW);
        gr716_set_pinfunc(22, IO_MODE_SPW);
        gr716_set_pinfunc(23, IO_MODE_SPW);
        gr716_set_pinfunc(24, IO_MODE_SPW);
}

/*
 * This function is called by BCC run-time setup before console initialization.
 *
 * Configures SpaceWire for use with GR716-BOARD and GR716-DEV
 */
void __bcc_init70(void)
{
        gr716_pll_config(PLL_REF_SPW_CLK, PLL_FREQ_12MHZ, PLL_POWER_ENABLE);
        gr716_spwclk(SPWCLK_SOURCE_PLL, 2, 0);
        gr716_set_lvdsfunc(LVDS_MODE_SPW);
        if (0) {
                /* for GRSPW2 port 1 */
                enable_the_grspw2_pins();
        }
        enable_the_grspw2_clock();
        if (1) {
                /* 10 MHz startup and run clock */
                volatile uint32_t *const thereg = (void *) 0x8010000c;
                *thereg = 0x0909;
        }
}

#else
/* non-empty translation unit to keep compiler happy */
void __bcc_initx(void) { }
#endif

