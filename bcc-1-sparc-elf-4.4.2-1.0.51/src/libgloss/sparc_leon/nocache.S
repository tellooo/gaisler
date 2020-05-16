/* Written by Konrad Eisele. (C) 2004  Gaisler Research AB */

#include <asm-leon/elfmacro.h>
#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/winmacros.h>
#include <asm-leon/leonbare_kernel.h>

	FUNC_EXPORT(leonbare_leon23_loadnocache)
	FUNC_EXPORT(leonbare_leon23_loadnocache16)
	FUNC_EXPORT(leonbare_leon23_loadnocache8)

	FUNC_EXPORT(leonbare_leon3_loadnocache)
	FUNC_EXPORT(leonbare_leon3_loadnocache16)
	FUNC_EXPORT(leonbare_leon3_loadnocache8)

	
	FUNC_EXPORT(leonbare_leon23_storenocache)
	FUNC_EXPORT(leonbare_leon23_storenocache16)
	FUNC_EXPORT(leonbare_leon23_storenocache8)
	
	FUNC_IMPORT(_leon_version)

	.text

/* =================================== */	
/* LEON2 / 3 */
/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon23_loadnocache)		/* use only %o0,%o1,%o7 */	
	lda 	[%o0] ASI_LEON3_CACHEMISS, %o0
	retl
	 nop
FUNC_END(leonbare_leon23_loadnocache)

/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon23_loadnocache16)
	lduha 	[%o0] ASI_LEON3_CACHEMISS, %o0
	retl
	 nop
FUNC_END(leonbare_leon23_loadnocache16)

/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon23_loadnocache8)
	lduba 	[%o0] ASI_LEON3_CACHEMISS, %o0
	retl
	 nop
FUNC_END(leonbare_leon23_loadnocache8)

/* write through cache */
FUNC_BEGIN(leonbare_leon23_storenocache)		/* use only %o0,%o1,%o7 */	
	st 	%o1, [%o0]
	retl
	 mov 	%o1,%o0
FUNC_END(leonbare_leon23_storenocache)
		
/* write through cache */
FUNC_BEGIN(leonbare_leon23_storenocache16)
	sth 	%o1, [%o0]
	retl
	 mov 	%o1,%o0
FUNC_END(leonbare_leon23_storenocache16)

/* write through cache */
FUNC_BEGIN(leonbare_leon23_storenocache8)
	stb 	%o1, [%o0]
	retl
	 mov 	%o1,%o0
FUNC_END(leonbare_leon23_storenocache8)



/* =================================== */	
/* LEON3 only */
/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon3_loadnocache)		/* use only %o0,%o1,%o7 */	
	retl
	 lda 	[%o0] ASI_LEON3_CACHEMISS, %o0
FUNC_END(leonbare_leon3_loadnocache)

/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon3_loadnocache16)
	retl
	 lduha 	[%o0] ASI_LEON3_CACHEMISS, %o0
FUNC_END(leonbare_leon3_loadnocache16)

/* load with forceing cache miss */	
FUNC_BEGIN(leonbare_leon3_loadnocache8)
	retl
	 lduba 	[%o0] ASI_LEON3_CACHEMISS, %o0
FUNC_END(leonbare_leon3_loadnocache8)
	

	
