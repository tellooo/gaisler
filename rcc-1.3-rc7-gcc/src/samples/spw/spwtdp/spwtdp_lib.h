/*  SPWTDP - SpaceWire Time Distribution Protocol. 
 *
 *  COPYRIGHT (c) 2017.
 *  Cobham Gaisler AB
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 */

#ifndef __SPWTDP_LIB_H__
#define __SPWTDP_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <grlib/spwtdp.h>

extern int spwtdp_precision_parse(spwtdp_time_t * a, int *coarse, int *fine);
extern int spwtdp_et_print(spwtdp_time_t * a, const char * msg);
extern int spwtdp_et_sub(spwtdp_time_t *a, spwtdp_time_t *b, spwtdp_time_t *r);
extern int spwtdp_et_add(spwtdp_time_t *a, spwtdp_time_t *b, spwtdp_time_t *r);
extern int spwtdp_et_cmp(spwtdp_time_t *a, spwtdp_time_t *b);
extern int spwtdp_et_to_uint(spwtdp_time_t *a, unsigned long long *coarse, unsigned long long *fine);
extern int spwtdp_et_split(spwtdp_time_t *a, uint8_t *coarse, uint8_t *fine);

#ifdef __cplusplus
}
#endif

#endif

