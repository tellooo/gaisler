.macro FUNC_BEGIN name
        .align 4
        .type \name, #function
        \name:
.endm

.macro FUNC_END name
        .size \name, .-\name
.endm

.equiv ASI_MMUBYPASS, 0x1c

	.section	".text"
        .global         mscrub_load_mmubypass
        .global         mscrub_casa_mmubypass


FUNC_BEGIN mscrub_load_mmubypass
        retl
         lda    [%o0] ASI_MMUBYPASS, %o0
FUNC_END mscrub_load_mmubypass


! Compare register %o1 with memory word at address %o0. If values are
! equal, %o1 and memory word at %o0 are swapped, atomically.
	.align	16
FUNC_BEGIN mscrub_casa_mmubypass
        casa    [%o0] ASI_MMUBYPASS, %o1, %o1
        retl
         nop
FUNC_END mscrub_casa_mmubypass

