/*
 * Software scrub library for AHBSTAT
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#include <stdint.h>
#include <mscrub.h>

struct ahbstat_regs {
        uint32_t status;
        uint32_t failing;
};

/* These are bit numbers for the AHBSTAT Status register. */
#define AHBSTAT_STATUS_CE_BIT           9
#define AHBSTAT_STATUS_NE_BIT           8
#define AHBSTAT_STATUS_HWRITE_BIT       7
#define AHBSTAT_STATUS_HMASTER_BIT      3
#define AHBSTAT_STATUS_HSIZE_BIT        0

/* These are bit masks for the AHBSTAT Status register. */
#define AHBSTAT_STATUS_CE               (1 << AHBSTAT_STATUS_CE_BIT)
#define AHBSTAT_STATUS_NE               (1 << AHBSTAT_STATUS_NE_BIT)
#define AHBSTAT_STATUS_HWRITE           (1 << AHBSTAT_STATUS_HWRITE_BIT)
#define AHBSTAT_STATUS_HMASTER          (0xf << AHBSTAT_STATUS_HMASTER_BIT)
#define AHBSTAT_STATUS_HSIZE            (0x7 << AHBSTAT_STATUS_HSIZE_BIT)

/*
 * COMPARE_AHBSTAT_FAILING
 * 0: Rewrite target word on correctable error, independently of
 * ahbstat->failing.
 * 1: Rewrite target word on correctable error only if target word address
 * matches ahbstat->failing.
 */
static const int COMPARE_AHBSTAT_FAILING = 0;

uint32_t mscrub_load_mmubypass(
        uint32_t addr
);
void mscrub_casa_mmubypass(
        uint32_t addr,
        uint32_t value
);
uint32_t mscrub_scrub(
        struct mscrub_info *info
)
{
        volatile struct ahbstat_regs *const ahbstat = info->ahbstat_regs;
        const uint32_t start = info->start;
        const uint32_t end = info->end;
        size_t blocksize = info->blocksize;
        uint32_t addr = info->next;
        uint32_t ret = 0;
        size_t i;

        for (i = 0; i < (blocksize / 4); i++) {
                uint32_t value;
                uint32_t status;
                uint32_t failing;

                /* Start AHBSTAT monitoring */
                ahbstat->status = 0;
                value = mscrub_load_mmubypass(addr);

                /* The read order of status and failing is important. */
                status = ahbstat->status;
                if (COMPARE_AHBSTAT_FAILING) {
                        failing = ahbstat->failing;
                } else {
                        failing = addr;
                }

                if (
                        (status & (AHBSTAT_STATUS_CE | AHBSTAT_STATUS_NE)) &&
                        (failing == addr)
                ) {
                        /* Correctable error detected: rewrite target */
                        mscrub_casa_mmubypass(addr, value);
                        ret++;
                }

                addr += 4;
                if (end <= addr) {
                        addr = start;
                }
        }
        info->nce += ret;

        /* Remember address for next round. */
        info->next = addr;

        return ret;
}

uint32_t mscrub_getnce(
        const struct mscrub_info *info
)
{
        return info->nce;
}

