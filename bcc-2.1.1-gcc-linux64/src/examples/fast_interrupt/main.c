/*
 * This example demonstrates how to implement a "fast interrupt" in BCC2.
 *
 * It installs a dedicated trap handler corresponding interrupt level 14
 * (MY_INT_LEVEL). This trap is made exclusive by bcc_set_trap() and overrides
 * any user ISR installed with bcc_isr_register(). The resulting fast interrupt
 * handler has very fast dispatch and exit.
 *
 * In this example the file myinthandler.S, written in assembly, implements the actual interrupt
 * handler code.
 *
 * It is important that the fast interrupt handler code
 * only uses the registers %l0 and %l3..%l7 and does not change window.
 *
 * To see the interrupt handler in action, set a breakpoint on symbol printf
 * with GRMON, and inspect the instruction trace:
 *   grmon2> load fast_interrupt.elf
 *   grmon2> bp hard printf
 *   grmon2> run
 *   [ execution breaks ]
 *   grmon2> cont
 *   Fast interrupt example. myregaddr=0x80000310
 *   [ execution breaks ]
 *   grmon2> inst 32
 *
 * Output of the inst command will contain an instruction with [  TRAP  ] in
 * the RESULT column. After this, execution continues in the trap table (offset
 * 0x1E0) and jumps to the interrupt handler at symbol myinthandler
 * (myinthandler.S). The interrupt handler ends with a "jmp %l1", "rett %l2"
 * sequence. Then the trapped instruction executes.
 */

#include <stdint.h>
#include <stdio.h>
#include <bcc/bcc.h>
#include <bcc/bcc_param.h>
#include <bcc/regs/gptimer.h>

struct gptimer_regs *gptimer0regs;
uint32_t *myregaddr = (void *) 0x80000000;
volatile uint32_t mylastval = 0;
volatile uint32_t mycount = 0;
extern void (myinthandler)(void);

#define INT_LEVEL_0 0x10
#define MY_INT_LEVEL 14

int main(void)
{
        /* Point to GPTIMER0, subtimer0 counter */
        gptimer0regs = (void *) __bcc_timer_handle;
        myregaddr = &gptimer0regs->timer[0].counter;
        printf("Fast interrupt example. myregaddr=%p\n", (void *) myregaddr);

        bcc_set_trap(INT_LEVEL_0 + MY_INT_LEVEL, myinthandler);
        bcc_flush_cache();

        bcc_int_clear(MY_INT_LEVEL);
        bcc_int_unmask(MY_INT_LEVEL);

        while (1) {
                for (volatile int i = 0; i < 1000000; i++) {
                        ;
                }
                /* Trig MY_INT_LEVEL once */
                bcc_int_pend(MY_INT_LEVEL);

                printf(
                        "mycount=%6u, mylastval=0x%08x\n",
                        (unsigned int) mycount,
                        (unsigned int) mylastval
                );
        }

        return 0;
}

