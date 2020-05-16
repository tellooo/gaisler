#include <stdio.h>
#include <stdlib.h>
#include <bcc/bcc.h>

static void tick(int n);

int main(void)
{
        puts("");
        puts("--- EXAMPLE BEGIN ---");

        tick(1000);

        puts("");
        puts("--- EXAMPLE END ---");

        return EXIT_SUCCESS;
}

static void tick(int n) {
        uint32_t t0;
        uint32_t tdiff;
        const uint32_t DURATION = 1000000;

        t0 = bcc_timer_get_us();
        for (int i = 1; i <= n; i++) {
                do {
                        uint32_t t1;
                        t1 = bcc_timer_get_us();
                        tdiff = t1 - t0;
                } while (tdiff <= DURATION);
                t0 += DURATION;

                printf("tick %3d | ", i);
                puts("");
        }
}

