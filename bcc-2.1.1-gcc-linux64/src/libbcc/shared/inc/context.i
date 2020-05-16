/*
 * Copyright (c) 2017, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. 
 */

.ifndef _CONTEXT_I_
_CONTEXT_I_ = 1

.include "macros.i"

! Standard register store area for window registers
        STRUCTDEF store_area
                WORD    l0
                WORD    l1
                WORD    l2
                WORD    l3
                WORD    l4
                WORD    l5
                WORD    l6
                WORD    l7

                WORD    i0
                WORD    i1
                WORD    i2
                WORD    i3
                WORD    i4
                WORD    i5
                WORD    i6
                WORD    i7
        ENDSTRUCT

        STRUCTDEF isr_ctx
                WORD    y
                WORD    g1
                WORD    g2
                WORD    g3
                WORD    g4
                WORD    _align
        ENDSTRUCT

        STRUCTDEF isr_ctx_flat
                WORD    y
                WORD    g1
                WORD    g2
                WORD    g3
                WORD    g4
                WORD    g5

                WORD    o0
                WORD    o1
                WORD    o2
                WORD    o3
                WORD    o4
                WORD    o5
                WORD    o6
                WORD    o7

                WORD    i6
                WORD    i7
        ENDSTRUCT

        STRUCTDEF fpu_state
                WORD    f0
                WORD    f1
                WORD    f2
                WORD    f3
                WORD    f4
                WORD    f5
                WORD    f6
                WORD    f7
                WORD    f8
                WORD    f9
                WORD    f10
                WORD    f11
                WORD    f12
                WORD    f13
                WORD    f14
                WORD    f15
                WORD    f16
                WORD    f17
                WORD    f18
                WORD    f19
                WORD    f20
                WORD    f21
                WORD    f22
                WORD    f23
                WORD    f24
                WORD    f25
                WORD    f26
                WORD    f27
                WORD    f28
                WORD    f29
                WORD    f30
                WORD    f31

                WORD    fsr
                WORD    _align
        ENDSTRUCT

.endif

