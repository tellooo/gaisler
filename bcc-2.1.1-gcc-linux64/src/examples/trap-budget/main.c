/*
 * Example on how to measure execution time for window_overflow and
 * window_underflow. Requires CPU upcounter in %asr22, %asr23.
 *
 * Author: Martin Åberg, Cobham Gaisler AB
 */

#include <stdio.h>
#include <bcc/bcc.h>

extern int meas_save_restore(unsigned int *thesave, unsigned int *therestore);
extern void meas_end(void);
int upcount_start(void);
/* takes about this many cycles */
#define UPCOUNT_GET_EXECTIME 6
unsigned int upcount_get(void);

int main(void)
{
        unsigned int s;
        unsigned int r;
        unsigned int tdis;
        unsigned int ten;

        printf("\nThis program measures run-time budgets in CPU cycles\n");
        upcount_start();
        meas_save_restore(&s, &r);

        {
                unsigned int t0;
                unsigned int t1;
                unsigned int t2;
                int plevel;

                t0 = upcount_get();
                plevel = bcc_int_disable();

                t1 = upcount_get();

                bcc_int_enable(plevel);

                t2 = upcount_get();

                tdis = t1 - t0 - UPCOUNT_GET_EXECTIME;
                ten = t2 - t1 - UPCOUNT_GET_EXECTIME;
        }
        meas_end();

        printf("  window_overflow:    %4lu\n", s);
        printf("  window_underflow:   %4lu\n", r);
        printf("  bcc_int_disable():  %4lu\n", tdis);
        printf("  bcc_int_enable():   %4lu", ten);
        printf("                      \n"); /* flush uart fifo */

        return 0;
}

