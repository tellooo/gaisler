/*
 * Verbose AMBA Plug&Play scanning
 *
 * This example demonstrates how the ambapp API can be used to list all AMBA
 * Plug&Play devices in the system. AHB masters/slaves and APB slaves are
 * listed. Bridges are recursed.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include <bcc/bcc_param.h>
#include <bcc/ambapp.h>

static int nfound = 0;
static const uint32_t MYDEPTH = 4;

static int pretty_ahbi(
        struct amba_ahb_info *ahbi,
        char *sp
)
{
        for (int j = 0; j < 4; j++) {
                if (0 == ahbi->bar[j].mask) {
                        continue;
                }
                printf(
                        "%s AHB: start=%08x, mask=%08x, type=%x\n",
                        sp,
                        ahbi->bar[j].start,
                        ahbi->bar[j].mask,
                        ahbi->bar[j].type
                );
        }

        return 0;
}

static int pretty_apbi(
        struct amba_apb_info *apbi,
        char *sp
)
{
        printf(
                "%s APB: start=%08x, mask=%08x, irq=%d, ver=%d\n",
                sp,
                apbi->start,
                apbi->mask,
                apbi->irq,
                apbi->ver
        );

        return 0;
}

static uint32_t v(
        void *info,
        uint32_t vendor,
        uint32_t device,
        uint32_t type,
        uint32_t depth,
        void *arg
)
{
        struct amba_ahb_info *ahbi = info;
        struct amba_apb_info *apbi = info;

        char sp[MYDEPTH+1];
        memset(&sp, ' ', MYDEPTH);
        sp[depth] = '\0';
        printf(
                "%sinfo=%08x, vendor=%02x, device=%03x, type=%x\n",
                sp,
                (unsigned int) info,
                vendor,
                device,
                type
        );
        if (type == AMBAPP_VISIT_AHBMASTER || type == AMBAPP_VISIT_AHBSLAVE) {
                pretty_ahbi(ahbi, &sp[0]);

        } else if (type == AMBAPP_VISIT_APBSLAVE) {
                pretty_apbi(apbi, &sp[0]);
        } else {
                printf("%s%s: Unknown type: %u\n", sp, __func__, type);
        }

        nfound++;

        return 0;
}

static uint32_t myapbuart(
        void *info,
        uint32_t vendor,
        uint32_t device,
        uint32_t type,
        uint32_t depth,
        void *arg
)
{
        struct amba_apb_info *apbi = info;

        return apbi->start;
}

int main(void)
{
        const uint32_t myioarea = 0xfff00000;
        uint32_t ret;
        clock_t t0, t1;

        puts("Find all AMBA Plug&Play devices.");
        printf("Starts at: 0x%08x\n", myioarea);
        ret = ambapp_visit(myioarea, 0, 0, 0xffffffff, MYDEPTH, v, NULL);
        puts ("");

        printf("found %d devices\n", nfound);
        puts("");

        for (int i = 0; i < 4; i++) {
                puts("");
                puts("Find first APBUART, measuring time with clock().");
                t0 = clock();
                ret = ambapp_visit(myioarea, VENDOR_GAISLER, GAISLER_APBUART, AMBAPP_VISIT_APBSLAVE, 4, myapbuart, NULL);
                t1 = clock();
                printf(
                        "%s: found at %08x after %u microseconds\n",
                        __func__,
                        ret,
                        (unsigned int) t1 - t0
                );
        }

        puts("");
        printf("%s: __bcc_con_handle            = %08x\n", __func__, __bcc_con_handle);
        printf("%s: __bcc_timer_handle          = %08x\n", __func__, __bcc_timer_handle);
        printf("%s: __bcc_timer_interrupt       = %d\n",   __func__, __bcc_timer_interrupt);
        printf("%s: __bcc_int_handle            = %08x\n", __func__, __bcc_int_handle);

        return 0;
}

