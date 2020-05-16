/*
 * Example on how to use the software scrub library "mscrub".
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */
#include <stdio.h>
#include <mscrub.h>
#include <bcc/ambapp.h>

#define GAISLER_AHBSTAT 0x052

/*
 * Example scrub context for GR712RC/UT699E/UT700/etc. 1 MiB of RAM starting at
 * 0x40100000 is scrubbed.
 */
struct mscrub_info mi = {
        .ahbstat_regs   = (void *) 0x80000f00,
        .start          = 0x40100000,
        .end            = 0x40200000,
        .blocksize      = 0x1000,
        .next           = 0x40100000,
        .nce            = 0
};

int main(void)
{
        uint32_t ahbstat_regs;
        static const uint32_t ioarea = 0xfff00000;

        ahbstat_regs = ambapp_visit(
                ioarea,
                VENDOR_GAISLER,
                GAISLER_AHBSTAT,
                AMBAPP_VISIT_APBSLAVE,
                4,
                ambapp_findfirst_fn,
                NULL
        );

        if (!ahbstat_regs) {
                printf("ERROR: AHBSTAT0 not found\n");
                exit(1);
        }
        mi.ahbstat_regs = (void *) ahbstat_regs;

        while (1) {
                uint32_t ret;
                uint32_t total;

                ret = mscrub_scrub(&mi);
                if (0 == ret) {
                        continue;
                }

                total = mscrub_getnce(&mi);
                printf(
                        "Detected %3u correctable errors (%3u in total).\n",
                        (unsigned int) ret,
                        (unsigned int) total
                );
        }

        return 0;
}

