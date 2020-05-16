;; Scheduling description for Leon.
;;   Copyright (C) 2002 Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING.  If not, write to
;; the Free Software Foundation, 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;; The Leon is a single-issue processor.

(define_automaton "leon")

(define_cpu_unit "leon_memory, leon_fpalu" "leon")
(define_cpu_unit "leon_fpmds" "leon")

(define_reservation "wb" "leon_memory")

(define_insn_reservation "leon_load" 1
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "load,sload,fpload"))
  "leon_memory, nothing")

(define_insn_reservation "leon_fp_alu" 1
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "fp,fpmove"))
  "leon_fpalu, nothing")

(define_insn_reservation "leon_fp_mult" 1
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "fpmul"))
  "leon_fpmds, nothing")

(define_insn_reservation "leon_fp_div" 16
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "fpdivs,fpdivd"))
  "leon_fpmds, nothing*15")

(define_insn_reservation "leon_fp_sqrt" 23
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "fpsqrts,fpsqrtd"))
  "leon_fpmds, nothing*21")


(define_insn_reservation "leon_ld" 1
  (and (eq_attr "cpu" "leon")
   (eq_attr "type" "load,sload"))
  "leon_memory")

(define_insn_reservation "leon_st" 1
  (and (eq_attr "cpu" "leon")
    (eq_attr "type" "store"))
  "leon_memory, wb")

