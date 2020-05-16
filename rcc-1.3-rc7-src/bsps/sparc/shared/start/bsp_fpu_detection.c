/**
 * @file
 * @ingroup sparc_bsp
 * @brief ERC32/LEON2/LEON3 BSP FPU run-time detection.
 */

/*
 *  COPYRIGHT (c) 2018.
 *  Cobham Gaisler AB.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#include <rtems/score/cpu.h>

#ifdef SPARC_DYNAMIC_FPU_DETECTION
int sparc_fpu_present;
#endif
