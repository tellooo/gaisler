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

.ifndef _MACROS_I_
_MACROS_I_ = 1

.macro FUNC_BEGIN name
        .global \name
        .align 4
        .type \name, #function
        \name:
.endm

.macro FUNC_END name
        .size \name, .-\name
.endm

.macro BITDEF name bit mask
        .equiv \name\()_BIT, \bit
        .equiv \name\(), (\mask << \name\()_BIT)
.endm

.macro STRUCTDEF sname
        sizeof_\sname = 0
        .macro ADDR fname
                .equiv \sname\()_\fname, sizeof_\sname
                .equ sizeof_\sname, sizeof_\sname + 4
        .endm
        .macro WORD fname
                .equiv \sname\()_\fname, sizeof_\sname
                .equ sizeof_\sname, sizeof_\sname + 4
        .endm
        .macro HALF fname
                .equiv \sname\()_\fname, sizeof_\sname
                .equ sizeof_\sname, sizeof_\sname + 2
        .endm
        .macro BYTE fname
                .equiv \sname\()_\fname, sizeof_\sname
                .equ sizeof_\sname, sizeof_\sname + 1
        .endm
        .macro STRUCT ftype fname
                .equiv \sname\()_\fname, sizeof_\sname
                .equ sizeof_\sname, sizeof_\sname + sizeof_\ftype
        .endm
.endm

.macro ENDSTRUCT
        .purgem ADDR
        .purgem WORD
        .purgem HALF
        .purgem BYTE
        .purgem STRUCT
.endm

! Example with nesting structs
!
!        STRUCTDEF example0
!                ADDR            ctx
!                WORD            state
!        ENDSTRUCT
!
!        STRUCTDEF example1
!                STRUCT example0 ex0
!                WORD            tos
!        ENDSTRUCT
.endif

