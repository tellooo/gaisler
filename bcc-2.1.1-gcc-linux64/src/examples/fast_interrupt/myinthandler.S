        .section        .text
        .global         myinthandler

myinthandler:
        /* FIXME: Check source and unpend */

        set     myregaddr, %l3
        /* Get register address */
        ld      [%l3], %l4
        /* Read register */
        ld      [%l4], %l5

        /* Store register value to user variable */
        set     mylastval, %l7
        st      %l5, [%l7]

        /* Update ISR count */
        sethi   %hi(mycount), %l3
        ld      [%l3 + %lo(mycount)], %l4
        add     %l4, 1, %l5
        st      %l5, [%l3 + %lo(mycount)]

        jmp     %l1
         rett   %l2

