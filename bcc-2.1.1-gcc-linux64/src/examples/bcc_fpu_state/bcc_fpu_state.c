/*
 * Exercise nested ISR and explicit FPU context save/restore.
 * NOTE: This example is not compatible with -msoft-float
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <bcc/bcc.h>

#define dbg(...) if (1) { \
        printf("%s:%d: ", __func__, __LINE__); \
        printf(__VA_ARGS__); \
}

enum {
        HISOURCE = 15
};

extern double force_with_float(double val, int source);
extern double ffun(int s, const char *spaces);
/*
 * This interrupt handler nests itself by forcing a higher interrupt level.
 */
void myhandler(void *arg, int source)
{
        int *a = arg;
        char spaces[HISOURCE + 1];
        double f0, f1, f0x;
        int nestcount;

        /* FPU context on stack for each ISR */
        struct bcc_fpu_state fpustate;

        bcc_fpu_save(&fpustate);

        nestcount = bcc_int_nestcount();
        memset(&spaces[0], ' ', HISOURCE + 1);
        spaces[nestcount] = '\0';

        f0 = ffun(source, spaces);
        dbg("%s[source=%2d, nest=%2d] f0=%4.2f before force\n", spaces, source, nestcount, f0);

        if (source < HISOURCE) {
                int nextsource = source + 1;

                dbg("%sforcing level=%d\n", spaces, nextsource);
                f0x = force_with_float(f0, nextsource);
        } else {
                dbg("%sno forcing by me\n", spaces);
                f0x = 999;
        }

        f1 = ffun(source, spaces);
        nestcount = bcc_int_nestcount();
        dbg("%s[source=%2d, nest=%2d] f0=%4.2f, f0x=%4.2f, f1=%4.2f after force\n", spaces, source, nestcount, f0, f0x, f1);
        *a = *a + 1;

        bcc_fpu_restore(&fpustate);
}

static volatile int myarg;

#ifndef _SOFT_FLOAT

int main(void)
{
        int ret;
        void *isr[HISOURCE+1];

        myarg = 0;
        dbg("myarg=%d\n", myarg);

        dbg("set pil = 0xf\n");
        ret = bcc_set_pil(0xf);
        assert(0 == ret);

        ret = bcc_int_enable_nesting();
        assert(BCC_OK == ret);

        dbg("register handler for source 1..%d\n", HISOURCE);
        for (int i = 1; i <= HISOURCE ; i++) {
                isr[i] = bcc_isr_register(i, myhandler, (void *) &myarg);
                assert(NULL != isr[i]);
        }

        dbg("unmask interrupt source 1..%d\n", HISOURCE);
        for (int i = 1; i <= HISOURCE ; i++) {
                ret = bcc_int_unmask(i);
                assert(BCC_OK == ret);
        }
        dbg("set pil = 0x0\n");
        ret = bcc_set_pil(0x0);
        assert(0xf == ret);

        dbg("unmasking source=1\n");
        ret = bcc_int_force(1);
        assert(BCC_OK == ret);

        dbg("unregister handler for source 1..%d\n", HISOURCE);
        for (int i = 1; i <= HISOURCE ; i++) {
                ret = bcc_int_unmask(i);
                ret = bcc_isr_unregister(isr[i]);
                assert(BCC_OK == ret);
        }

        return 0;
}

#else

int main(void)
{
        printf("Example %s is not compatible with soft float.\n", __FILE__);
	dbg("");
}

#endif

