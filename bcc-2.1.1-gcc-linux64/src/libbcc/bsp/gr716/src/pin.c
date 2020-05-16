#include <stdint.h>
#include <bcc/gr716/pin.h>
#include <bcc/bsp_pnp.h>
#include <bcc/gr716/grpll0_regs.h>

int gr716_set_pinfunc(unsigned int pin, unsigned int mode)
{
        if (64 <= pin) {
                return 1;
        }
        if (IO_MODE_MAX < mode) {
                return 2;
        }
        /* There are 8 4-bit fields per control register. */
        const int regi  = pin / 8;
        const int field = pin % 8;
        const int shift = field * 4;
        const uint32_t mask = 0xf  << shift;
        volatile uint32_t *const gpio = (void *) GAISLER_GPREGBANK_0_PNP_APB;
        uint32_t val;

        val = gpio[regi];
        val &= ~mask;
        val |= (mode << shift);
        gpio[regi] = val;

        return 0;
}

#define MYLVDS(rx2, rx1, rx0, tx2, tx1, tx0) \
  (((rx2) << 20) | ((rx1) << 16) | (rx0 << 12) | (tx2 << 8) | (tx1 << 4) | (tx0 << 0))

static const uint32_t GR716_LVDS_FUNCTOVAL[4] = {
/*                         rx2              rx1              rx0              tx2              tx1             tx0 */
[LVDS_MODE_SPW]   = MYLVDS(LVDS_DISABLE,    LVDS_MODE_SPW,   LVDS_MODE_SPW,   LVDS_DISABLE,    LVDS_MODE_SPW,  LVDS_MODE_SPW),
[LVDS_MODE_SPI4S] = MYLVDS(LVDS_MODE_SPI4S, LVDS_MODE_SPI4S, LVDS_MODE_SPI4S, LVDS_MODE_SPI4S, LVDS_DISABLE,   LVDS_DISABLE),
[LVDS_MODE_SPIS]  = MYLVDS(LVDS_MODE_SPIS,  LVDS_MODE_SPIS,  LVDS_MODE_SPIS,  LVDS_MODE_SPIS,  LVDS_DISABLE,   LVDS_DISABLE),
[LVDS_MODE_SPIM]  = MYLVDS(LVDS_DISABLE,    LVDS_DISABLE,    LVDS_MODE_SPIM,  LVDS_MODE_SPIM,  LVDS_MODE_SPIM, LVDS_MODE_SPIM),
};
#undef MYLVDS

static const uint32_t GR716_LVDS_FUNCDISABLE = 0x00888888;

/*
 * NOTE: the use of LVDS_MODE_ for two different things: register value and
 * user parameter.
 */
int gr716_set_lvdsfunc(unsigned int iofunc)
{
        volatile uint32_t *const lvdsreg =
            (void *) (GAISLER_GPREGBANK_0_PNP_APB + 0x30);

        if (iofunc <= 3) {
                *lvdsreg = GR716_LVDS_FUNCTOVAL[iofunc];
        } else if (iofunc == LVDS_DISABLE) {
                *lvdsreg = GR716_LVDS_FUNCDISABLE;
        } else {
                return 1;
        }
        return 0;
}

