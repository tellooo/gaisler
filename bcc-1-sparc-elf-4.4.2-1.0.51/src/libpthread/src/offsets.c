#define PTHREAD_KERNEL
#include "internals.h"
#include "setjmp.h"

#define DEFINE(sym, val) \
	asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define BLANK() asm volatile("\n->" : : )

main()
{

  jmp_buf env;
  struct context_t *scp;

  /* Offsets in the TCB */
#ifndef C_CONTEXT_SWITCH  
  /* Offsets in the context structure */
  DEFINE(AOFF_sp_offset, offsetof(struct pthread, context[THREAD_JB_SP]));
  DEFINE(AOFF_pc_offset, offsetof(struct pthread, context[THREAD_JB_PC]));
  DEFINE(AOFF_thread_errno, offsetof(struct pthread, terrno));
  DEFINE(AOFF_stack_base, offsetof(struct pthread, stack_base));
  DEFINE(AOFF_state, offsetof(struct pthread, state));
  DEFINE(AOFF_nscp, offsetof(struct pthread, nscp));
#endif
  DEFINE(AOFF_mask, offsetof(struct pthread, mask));
#ifndef C_CONTEXT_SWITCH  
  DEFINE(AOFF_pending, offsetof(struct pthread, pending));
#endif
#ifdef STAND_ALONE
  DEFINE(AOFF_queue, offsetof(struct pthread, queue));
  DEFINE(AOFF_tp_sec, offsetof(struct pthread, tp.tv_sec));
  DEFINE(AOFF_tp_nsec, offsetof(struct pthread, tp.tv_nsec));
#endif
  
  /* Offsets in kernel structure */
  DEFINE(AOFF_pthread_self, offsetof(struct kernel, k_pthread_self));
#ifndef C_CONTEXT_SWITCH
  DEFINE(AOFF_is_in_kernel, offsetof(struct kernel, k_is_in_kernel));
  DEFINE(AOFF_is_updating_timer, offsetof(struct kernel, k_is_updating_timer));
  DEFINE(AOFF_state_change, offsetof(struct kernel, k_state_change));
  DEFINE(AOFF_new_signals, offsetof(struct kernel, k_new_signals));
  DEFINE(AOFF_pending_signals, offsetof(struct kernel, k_pending_signals));
  DEFINE(AOFF_all_signals, offsetof(struct kernel, k_all_signals));
  DEFINE(AOFF_no_signals, offsetof(struct kernel, k_no_signals));
#endif
  DEFINE(AOFF_cantmask, offsetof(struct kernel, k_cantmask));
#ifndef C_CONTEXT_SWITCH
  DEFINE(AOFF_process_stack_base, offsetof(struct kernel, k_process_stack_base));
  DEFINE(AOFF_ready, offsetof(struct kernel, k_ready));
  DEFINE(AOFF_ready_head, offsetof(struct kernel, k_ready.head));
  DEFINE(AOFF_TV_SEC, offsetof(struct kernel, k_timeofday.tv_sec));
  DEFINE(AOFF_TV_NSEC, offsetof(struct kernel, k_timeofday.tv_nsec));

  /* Offsets in pthread_attr_t structure */
  DEFINE(AOFF_sched, offsetof(pthread_attr_t, sched));

  /* Offsets in context_t structure */
  DEFINE(AOFF_sc_mask, offsetof(struct context_t, sc_mask));
  DEFINE(AOFF_sc_sp, offsetof(struct context_t, sc_sp));
  DEFINE(AOFF_sc_pc, offsetof(struct context_t, sc_pc));
  
#endif

  /* Offsets in jmp_buf structure */
  DEFINE(AOFF_jmp_sp, (THREAD_JB_SP * sizeof(int)));
  DEFINE(AOFF_jmp_pc, (THREAD_JB_PC * sizeof(int)));
  DEFINE(AOFF_jmp_svmask, (THREAD_JB_SVMASK * sizeof(int)));
  DEFINE(AOFF_jmp_mask, (THREAD_JB_MASK * sizeof(int)));
  DEFINE(AOFF_jmp_fp, (THREAD_JB_FP * sizeof(int)));
  DEFINE(AOFF_jmp_i7, (THREAD_JB_I7 * sizeof(int)));

  /* Offsets in pthread_mutex_t structure */
  DEFINE(AOFF_mutex_queue, offsetof(pthread_mutex_t, queue));
  DEFINE(AOFF_mutex_lock, offsetof(pthread_mutex_t, lock));
  DEFINE(AOFF_mutex_owner, offsetof(pthread_mutex_t, owner));
#ifdef _POSIX_THREADS_PRIO_PROTECT
  DEFINE(AOFF_mutex_protocol, offsetof(pthread_mutex_t, protocol));
#endif
  
  /* Size of cleanup structure */
  DEFINE(ASIZ_cleanup_size,	sizeof(pthread_cleanup_t));

  /* Size of sigset_t structure */
  DEFINE(ASIZ_sigset_t_size,	sizeof(sigset_t));

  /* from config_header.c */
  DEFINE(PTHREAD_SIGSET_T_SIZE_NP,	sizeof(sigset_t));
  DEFINE(PTHREAD_SIGCONTEXT_MASK_T_SIZE_NP,sizeof(scp->sc_mask));
  DEFINE(PTHREAD_SIGSET2SET_SIZE_NP, MIN(sizeof(sigset_t), sizeof(scp->sc_mask)));
  
  return(0);
}
