/* NOTE: the lda should be on offset 0x18 */
#if defined(__FIX_LEON3FT_TN0018)

/*
 * l3: (out) original cctrl
 * l4: (out) original cctrl with ics=0
 * NOTE: This macro modifies psr.icc.
 */
.macro TN0018_WAIT_IFLUSH
1:
        ! wait for pending iflush to complete
        lda     [%g0] ASI_CTRL, %l3
        srl     %l3, CCTRL_IP_BIT, %l4
        andcc   %l4, 1, %g0
        bne     1b
         andn   %l3, CCTRL_ICS, %l4
.endm


.macro TN0018_WRITE_PSR src
        wr      \src, %psr
.endm

/*
 * l3: (in) original cctrl
 * l4: (in) original cctrl with ics=0
 * NOTE: This macro MUST be immediately followed by the "jmp;rett" pair.
 */
.macro TN0018_FIX
        .align  0x20                    ! align the sta for performance
        sta     %l4, [%g0] ASI_CTRL     ! disable icache
        nop                             ! delay for sta to have effect on rett
        or      %l1, %l1, %l1           ! delay + catch rf parity error on l1
        or      %l2, %l2, %l2           ! delay + catch rf parity error on l2
        sta     %l3, [%g0] ASI_CTRL     ! re-enable icache after rett
        nop                             ! delay ensures insn after gets cached
.endm

#else

.macro TN0018_WAIT_IFLUSH
.endm

.macro TN0018_WRITE_PSR src
.endm

.macro TN0018_FIX
.endm

#endif

