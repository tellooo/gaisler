#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/winmacros.h>

/* This asm code relies on the following offsets (setjmp.h): 
#define THREAD_JB_SP     0 
#define THREAD_JB_PC     1
#define THREAD_JB_PSR    8 
#define THREAD_JB_WIM    9 */

	/* Number of register windows */
	.global _nwindows_min1, _nwindows

	.text
	.global _switch_to
_switch_to:	
  
#ifndef _FLAT
        !mov     %o0,%i0                   /* propagate env on restore */                        
        !mov     %o1,%i1                   /* propagate val on restore */                        
        !restore                                                                        
        mov     %o0,%g7                                                              
        mov     %o1,%g3                   /* former %%i1 (val) */                           
	
	sethi %hi(_nwindows_min1), %g4    /* flush registers */
	ld [%g4+%lo(_nwindows_min1)], %g4
1:	save   %sp, -SF_REGS_SZ, %sp	 !NWINDOWS-1 times
	sub    %g4,1,%g4
	cmp    %g0,%g4
	bne    1b
	 nop
#else
        mov     %o0,%g7                                                              
        mov     %o1,%g3                   /* former %%i1 (val) */
	RW_STORE(sp)
#endif
	
        ldd     [%g7+THREAD_JB_PSR*4], %g4 /* load psr,wim */                                
        wr      %g4, 0x20, %psr                                                      
        nop                                                                            
        nop                                                                            
        nop                                                                            
        ldd     [%g7 +THREAD_JB_SP*4], %sp /* load sp, pc to jump to */                      
        wr      %g5, 0x0, %wim                                                       
        
	RW_LOAD(sp)                       /* restore window */ 
        wr      %g4, 0x00, %psr                                                      
        nop                                                                            
        nop                                                                            
        nop                                                                            
                                                                                       
        jmp     %o7 + 8                   /* success      */                                
        mov     %g3, %o0                  /* return %%g3  */                                
	
 

        .text                                    
	.global thread_setjmp, _do_thread_setjmp 
thread_setjmp:                                  

#ifdef _FLAT
	B2BSTORE_FIX
	B2BSTORE_FIX
	RW_STORE(sp)                      /* store window */
#endif

!	RW_STORE(sp)                      /* store window for _FLAT and normal, SWITCH_TO_STACK in pthread need this*/
		
	mov %psr,%o2                            
#ifndef _SOFT_FLOAT
        set 0x1000,%o3                          
        andn %o2,%o3,%o2      !disable fpu
#endif	
        std %sp,[%o0]         !THREAD_JB_SP     
        B2BSTORE_FIX
        st  %o2,[%o0+(8*4)]   !THREAD_JB_PSR    
        ba _do_thread_setjmp                    
         nop
