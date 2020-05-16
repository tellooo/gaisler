#ifndef _PT_DEBUG_H
#define _PT_DEBUG_H

#include <asm-leon/leondbg.h>

#define DEBUG_PT
//#define PDEBUG_FLAGS __pthread_dbgflags
#define PDEBUG_FLAGS (0)
#define PDEBUG_DO_ASSERT 

extern volatile int pthread_started;
#ifdef PDEBUG_DO_ASSERT
#define PDEBUG_FLAGS_check(code) ((PDEBUG_FLAGS & (code)) && pthread_started)
#else
#define PDEBUG_FLAGS_check(code) (((PDEBUG_FLAGS & (code)) || ((code) & PDBG_ASSERT)) && pthread_started)
#endif

#define PDBG_PRINTF dbgleon_printf

#define PDEBUG_PTNAME(code,p)                          \
    if ((p) && (p)->pt_name) {                         \
      PDEBUG_SIMPLECODE(code, "%s",(p)->pt_name);      \
    } else {                                           \
      PDEBUG_SIMPLECODE(code, "0x%08x",p);             \
    }                                                  

#define PDEBUG_HEADER(code)                            \
  if (PDEBUG_FLAGS_check(code)) {                      \
    register unsigned int _GETSP asm("sp");            \
    PDBG_PRINTF("[sp:%08x self(%08x):", _GETSP, mac_pthread_self()); \
    if ((mac_pthread_self()) && (mac_pthread_self())->pt_name) {\
      PDBG_PRINTF("%8s",mac_pthread_self()->pt_name);       \
    } else {                                           \
      PDBG_PRINTF("%08x",mac_pthread_self());               \
    }                                                  \
    PDBG_PRINTF(" %20s : %03d @ %28s()]:" , __FILE__,__LINE__,__FUNCTION__); \
  }                                                    
  
#define PDEBUG_CODE(code, fmt, args...) do { \
  PDEBUG_HEADER(code)                        \
  if (PDEBUG_FLAGS_check(code)) { \
    PDBG_PRINTF(fmt "\n" ,## args ); \
  } \
} while(0)

#define PDEBUG_SIMPLECODE(code, fmt, args...) do { \
  if (PDEBUG_FLAGS_check(code)) { \
    PDBG_PRINTF(fmt , ## args ); \
  } \
} while(0)

/* define the PDEBUG macro here */
#ifdef DEBUG_PT
#  define PDEBUG(code, fmt, args...) PDEBUG_CODE(code, fmt, ## args)
#else
#  define PDEBUG(code, fmt, args...) do { } while(0) /* debug switched off */
#endif

#define PDBG_FNCALL 1
#define PDBG_INIT  (1<<1)
#define PDBG_ALLOC (1<<2)
#define PDBG_STACK (1<<3)
#define PDBG_RUN   (1<<4)
#define PDBG_START (1<<5)
#define PDBG_SCHED (1<<6)
#define PDBG_QUEUE (1<<7)
#define PDBG_TIMER (1<<8)
#define PDBG_ERROR (1<<9)
#define PDBG_TIMEOUT (1<<10)
#define PDBG_ASSERT (1<<11)
#define PDBG_SIGNAL (1<<12)
#define PDBG_LIBC (1<<13)
#define PDBG_MUTEX (1<<14)
#define PDBG_RESCHED (1<<15)

#define PTRACEIN PDEBUG(PDBG_FNCALL,"->");
#define PTRACEOUT PDEBUG(PDBG_FNCALL,"<-");

#define PDEBUG_PTQUEUE(code,q)                         \
  if ((q) && (q)->tqh_name) {                          \
    PDEBUG_SIMPLECODE(code, "%s: [",(q)->tqh_name);    \
  } else {                                             \
    PDEBUG_SIMPLECODE(code, "0x%08x: [",(q));          \
  }                                                    

#define PDEBUG_PTQUEUELIST(code,q,index) do {          \
  pthread_t p = 0;                                     \
  PDEBUG_PTQUEUE(code,q)                               \
  TAILQ_FOREACH(p, (q), pt_qelem[index]) {             \
    if (p != TAILQ_FIRST(q)) {                         \
      PDEBUG_SIMPLECODE(code, " ");                    \
    }                                                  \
    PDEBUG_PTNAME(code,p)                              \
  }                                                    \
  PDEBUG_SIMPLECODE(code, "]\n");                      \
} while(0)

#define PTRACE_QUEUE(code,q,index) do {                \
  PDEBUG_HEADER(code)                                  \
  PDEBUG_PTQUEUELIST(code,q,index);                    \
} while(0)

#define PTRACE_READYQUEUE PTRACE_QUEUE(PDBG_QUEUE,&ready,PRIMARY_QUEUE)
#define PTRACE_ALLQUEUE PTRACE_QUEUE(PDBG_QUEUE,&all,K_QUEUES_ALL)
#define PTRACE_SUSPENDQUEUE PTRACE_QUEUE(PDBG_QUEUE,&suspend_q,PRIMARY_QUEUE)
#define PTRACE_TIMEOUTQUEUE PTRACE_QUEUE(PDBG_QUEUE,&pthread_timeout_q,K_QUEUES_TIMEOUT)

#define PTRACE_QUEUES  \
   PTRACE_READYQUEUE;  \
   PTRACE_ALLQUEUE;    \
   PTRACE_SUSPENDQUEUE;


#define PDEBUG_PTCOND(code,c)                          \
  if ((c) && (c)->dbgname) {                           \
    PDEBUG_SIMPLECODE(code, "%s: ",(c)->dbgname);      \
  } else {                                             \
    PDEBUG_SIMPLECODE(code, "0x%08x: ",(q));           \
  }                                                    

#define PDEBUG_COND(code,c)                            \
  PDEBUG_PTCOND(code,c);                               \
  PDEBUG_SIMPLECODE(code, "\n queue: ",(c)->dbgname);  \
  PDEBUG_PTQUEUELIST(code,&((c)->queue),PRIMARY_QUEUE);
  

#define PDEBUG_PTMUTEX(code,m)                         \
  if ((m) && (m)->dbgname) {                           \
    PDEBUG_SIMPLECODE(code, "%s: ",(m)->dbgname);      \
  } else {                                             \
    PDEBUG_SIMPLECODE(code, "0x%08x: ",(m));           \
  }                                                    

#define PDEBUG_MUTEX(code,m)                           \
  PDEBUG_PTMUTEX(code,m);                              \
  PDEBUG_SIMPLECODE(code, "\n owner: ");               \
  PDEBUG_PTNAME(code,(m)->owner);                      \
  PDEBUG_SIMPLECODE(code, "\n queue: ");               \
  PDEBUG_PTQUEUELIST(code,&((c)->queue),PRIMARY_QUEUE);


/* Define ASSERT to stop/warn. Should be void in production code */
#ifdef PDEBUG_DO_ASSERT
#  define PASSERT(x) if (!(x)) { PDEBUG(PDBG_ASSERT,"#### assertion failed "); while(1); }
#else
#  define PASSERT(x) do { } while(0)
#endif

#endif /* _PT_DEBUG_H */
