        .text
        .align 4
        .global _leon_tn0018_rett

#define THE_ASI_CTRL 2
#define THE_CCTRL_IP_BIT 15
#define THE_CCTRL_IP (0x1 << THE_CCTRL_IP_BIT)
#define THE_CCTRL_ICS (0x3 <<  0)
_leon_tn0018_rett:
1:
        ! wait for pending iflush to complete
        lda     [%g0] THE_ASI_CTRL, %l3
        srl     %l3, THE_CCTRL_IP_BIT, %l4
        andcc   %l4, 1, %g0
        bne     1b
         andn   %l3, THE_CCTRL_ICS, %l4

        wr      %l0, 0x0, %psr
        nop
        nop
        nop

        .align  0x20                    ! align the sta for performance
        sta     %l4, [%g0] THE_ASI_CTRL ! disable icache
        nop                             ! delay for sta to have effect on rett
        or      %l1, %l1, %l1           ! delay + catch rf parity error on l1
        or      %l2, %l2, %l2           ! delay + catch rf parity error on l2
        sta     %l3, [%g0] THE_ASI_CTRL ! re-enable icache after rett
        nop                             ! delay ensures insn after gets cached

        jmp     %l1
         rett   %l2

