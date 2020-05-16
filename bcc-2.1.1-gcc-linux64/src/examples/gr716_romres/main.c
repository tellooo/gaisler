#include <stdio.h>
#include <stdlib.h>
#include <bcc/bcc.h>

/*
 * Set the variable __bsp_sysfreq to the GR716 system clock frequency in Hz.
 *
 * The following initialization is for GR716 clocked at 20 MHz.
 */
const unsigned int __bsp_sysfreq = 20 * 1000 * 1000;

static void tick(int n);

int main(void)
{
        puts("--- EXAMPLE BEGIN ---");

        printf("\nHello GR716!\n\n");

        tick(10);

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
                printf("tick %d\n", i);
        }
}

