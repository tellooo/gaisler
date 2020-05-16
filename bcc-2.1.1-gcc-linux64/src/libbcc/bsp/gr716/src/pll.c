#include <stdint.h>
#include <bcc/gr716/pll.h>
#include <bcc/bsp_pnp.h>
#include <bcc/gr716/grpll0_regs.h>

static struct grpll0_regs *GETREGS(void)
{
        const uintptr_t REG_ADDR = GAISLER_GRPLL_0_PNP_APB;
        struct grpll0_regs *const regs = (struct grpll0_regs *) REG_ADDR;
        return regs;
}

int gr716_sysclk(int source, int div, int duty)
{
        if (source < CLK_SOURCE_CLK) {
                return 1;
        } else if (source > CLK_SOURCE_PLL) {
                return 2;
        } else if ((div != 0) && (duty != 0) && (duty <= div)) {
                return 3;
        }

        volatile struct grpll0_regs *const regs = GETREGS();
        regs->sysref = ((duty & 0xFF) << 16) | ((source & 0x3) << 8) | ((div & 0xFF) << 0);
        /* Select system ref clock if no division */
        if ((source == CLK_SOURCE_CLK) && (div == 0)) {
                regs->syssel = 0;
        } else {
                regs->syssel = 1;
        }
        return 0;
}

int gr716_spwclk(int source, int div, int duty)
{
        if (source < SPWCLK_SOURCE_CLK) {
                return 1;
        } else if (source > SPWCLK_SOURCE_PLL) {
                return 2;
        } else if ((div != 0) && (duty != 0) && (duty <= div)) {
                return 3;
        }

        volatile struct grpll0_regs *const regs = GETREGS();
        regs->spwref = ((duty & 0xFF) << 16) | ((source & 0x3) << 8) | ((div & 0xFF) << 0);
        return 0;
}

int gr716_pll_config(int ref, int ucfg, int pd)
{
        if (CLK_SOURCE_SPW < ref ) {
                return 1;
        }
        if (PLL_FREQ_5MHZ < ucfg) {
                return 2;
        }

        volatile struct grpll0_regs *const regs = GETREGS();
        uint32_t pllcfg;
        uint32_t pllref;

        pllcfg  = (pd << GRPLL0_CFG_PD_BIT) & GRPLL0_CFG_PD;
        pllcfg |= (ucfg << GRPLL0_CFG_CFG_BIT) & GRPLL0_CFG_CFG;
        pllref  = (ref << GRPLL0_PLLREF_SEL_BIT) & GRPLL0_PLLREF_SEL;

        regs->cfg = pllcfg;
        regs->pllref = pllref;
        return 0;
}

int gr716_pll_islocked(void)
{
        volatile struct grpll0_regs *const regs = GETREGS();

        return !!(regs->sts & GRPLL0_STS_CL);
}

