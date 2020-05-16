#ifndef _INCLUDE_LEON_ASMMACRO_h
#define _INCLUDE_LEON_ASMMACRO_h

#include <asm-leon/leonstack.h>

/* All trap entry points _must_ begin with this macro or else you
 * lose.  It makes sure the kernel has a proper window so that
 * c-code can be called.
 */
#define SAVE_ALL_HEAD \
	sethi	%hi(leonbare_trapsetup), %l4; \
	jmpl	%l4 + %lo(leonbare_trapsetup), %l6;
#define SAVE_ALL \
	SAVE_ALL_HEAD \
	 nop;

#define SAVE_ALL_FAST(l) \
        set     l-8, %l6; \
	sethi	%hi(leonbare_trapsetup_fast), %l4; \
	jmpl	%l4 + %lo(leonbare_trapsetup_fast), %g0; \
	 nop;

/* All traps low-level code here must end with this macro. */
#define RESTORE_ALL b leonbare_trapreturn; clr %l6;
#define RESTORE_ALL_FAST b leonbare_trapreturn_fast; clr %l6;

#define WRITE_PAUSE nop; nop; nop;

#endif /* !_INCLUDE_LEON_STACK_h */


