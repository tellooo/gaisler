        .global meas_save_restore
        .global meas_end
        .global upcount_start
        .global upcount_get
        .section        ".text"

meas_save_restore:
        save    %sp, -96, %sp

        set     32, %g3
        mov     %g3, %g2
1:
        save    %sp, -96, %sp
        cmp     %g3, %g0
        bne     1b
         sub    %g3, 1, %g3

        /* save exec time in %g6 */
        rd      %asr23, %g4
        save    %sp, -96, %sp
        rd      %asr23, %g6
        sub     %g6, %g4, %g6

        nop; nop; nop; nop
meas0:
        nop; nop; nop; nop

2:
        restore
        cmp     %g2, %g0
        bne     2b
         sub    %g2, 1, %g2

        /* restore exec time in %g7 */
        rd      %asr23, %g4
        restore
        rd      %asr23, %g7
        sub     %g7, %g4, %g7

        nop; nop; nop; nop
meas1:
        nop; nop; nop; nop

        st      %g6, [%i0]
        st      %g7, [%i1]

blupp:
        ret
        restore

meas_end:
        retl
         nop

upcount_start:
        wr      %g0, %asr22
        retl
         mov    %g0, %o0

upcount_get:
        retl
         rd     %asr23, %o0

