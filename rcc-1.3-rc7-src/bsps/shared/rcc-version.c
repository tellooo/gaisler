/*
 *  COPYRIGHT (c) 2019, Cobham Gaisler
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#include <bspopts.h>
#include <rtems/rcc.h>

#ifdef RCC_VERSION
const char _RCC_version[] = "rcc-" RCC_VERSION;
#else
const char _RCC_version[] = "";
#endif

const char *rcc_version(void)
{
	return _RCC_version;
}
