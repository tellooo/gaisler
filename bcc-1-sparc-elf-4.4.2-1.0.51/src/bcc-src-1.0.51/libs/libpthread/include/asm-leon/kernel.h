#ifndef H_KERNEL_H
#define H_KERNEL_H

#define LREG_KBASE %g7

#ifndef  __ASSEMBLER__

register unsigned int lreg_kbase asm("%g7");
register unsigned int lreg_sp asm("%sp");



#endif

#endif


