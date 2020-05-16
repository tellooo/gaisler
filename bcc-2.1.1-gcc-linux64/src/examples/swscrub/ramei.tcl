# Inject single bit error at specified memory location
proc ramei {addr} {
        set ::mctrl0::mcfg3::rb 1
        set data [mem $addr 4]
        set ::mctrl0::mcfg3::rb 0

        set tcb $::mctrl0::mcfg4
        set tcb [expr {$tcb & 0xffff}]
        # Single error
        set tcb [expr {$tcb ^ 1}]
        set tcb [expr {$tcb | (1<<16)}]
        set tcb [format "0x%08x" $tcb]
        set ::mctrl0::mcfg4 $tcb

        set data [format "0x%08x" $data]
        wmem $addr $data
        set ::mctrl0::mcfg3::wb 0
}

