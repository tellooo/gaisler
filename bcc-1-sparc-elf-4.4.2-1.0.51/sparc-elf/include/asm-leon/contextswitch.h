#ifndef H_LEONBARE_CONTEXTSWITCH_H
#define H_LEONBARE_CONTEXTSWITCH_H


/*
 * for this version, the index of THREAD_JB_SP must be even !!!
 * This way, we can speed up the context switch (using std).
 */
#define THREAD_JB_SP     0 /* aligned */
#define THREAD_JB_PC     1
#define THREAD_JB_SVMASK 3
#define THREAD_JB_MASK   4
#define THREAD_JB_FP     5
#define THREAD_JB_I7     6

#define THREAD_JB_PSR    8 /* aligned */
#define THREAD_JB_WIM    9

#define THREAD_JB_FPUCTX 10

#ifndef __ASSEMBLER__

extern unsigned long fpustate_current;

typedef int threadctx_t[14+2] __attribute__ ((aligned (8)));

int thread_setjmp(threadctx_t env, int val);
void thread_longjmp(threadctx_t env, int val);
void _switch_to(threadctx_t env, int val);

#endif /* __ASSEMBLER__ */

#endif


