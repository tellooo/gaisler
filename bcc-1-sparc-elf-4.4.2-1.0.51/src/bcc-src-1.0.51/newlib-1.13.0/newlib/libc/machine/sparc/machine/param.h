/* Various stuff for the sparclet processor.

   This file is in the public domain.  */

#ifndef _MACHINE_PARAM_H_
#define _MACHINE_PARAM_H_

#ifdef __leonbare__
/* note: this is also defined in asm-leon/param.h */
# define HZ		   100UL      /* Internal kernel timer frequency */
# define  CLOCK_TICK_RATE  1000000UL  /* Underlying HZ */
#endif

#endif /* _MACHINE_SPARCLET_H_ */
