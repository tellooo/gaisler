/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#ifndef _SYS_STACK_H_
#define	_SYS_STACK_H_

#if !defined(_ASM)
#include <sys/types.h>
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * A stack frame looks like:
 *
 * %fp->|				|
 *	|-------------------------------|
 *	|  Locals, temps, saved floats	|
 *	|-------------------------------|
 *	|  outgoing parameters past 6	|
 *	|-------------------------------|-\
 *	|  6 words for callee to dump	| |
 *	|  register arguments		| |
 *	|-------------------------------|  > minimum stack frame
 *	|  One word struct-ret address	| |
 *	|-------------------------------| |
 *	|  16 words to save IN and	| |
 * %sp->|  LOCAL register on overflow	| |
 *	|-------------------------------|-/
 */

/*
 * Constants defining a 32-bit stack frame.
 */
#define	WINDOWSIZE	(16*4)		/* size of window save area */
#define	ARGPUSHSIZE	(6*4)		/* size of arg dump area */
#define	ARGPUSH	        (WINDOWSIZE + 4)	/* arg dump area offset */
#define	MINFRAME	(WINDOWSIZE + ARGPUSHSIZE + 4) /* min frame */

#define	STACK_GROWTH_DOWN /* stacks grow from high to low addresses */

/*
 * Stack alignment macros.
 */
#define	STACK_ALIGN	8
#define	SA(X)		(((X)+(STACK_ALIGN-1)) & ~(STACK_ALIGN-1))

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_STACK_H */
