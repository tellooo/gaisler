/*
 *  This set of routines starts the application.  It includes application,
 *  board, and monitor specific initialization and configuration.
 *  The generic CPU dependent initialization has been performed
 *  before any of these are invoked.
 *
 *  COPYRIGHT (c) 1989-2007.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  Modified for LEON3 BSP.
 *  COPYRIGHT (c) 2004.
 *  Gaisler Research.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id: bspstart.c,v 1.15 2008/09/15 00:23:55 joel Exp $
 */


#include <bsp.h>
#include <rtems/bspIo.h>

/*
 * Tells us if data cache snooping is available
 */ 
int CPU_SPARC_HAS_SNOOPING;

/* LEON3 BSP Idle body */
extern void _BSP_Thread_Idle_body ( void );

/*
 * set_snooping
 * 
 * Read the data cache configuration register to determine if
 * bus snooping is available. This is needed for some drivers so
 * that they can select the most efficient copy routines.  
 *
 */

static inline int set_snooping(void) 
{
        int tmp;        
        asm(" lda [%%g0] 2, %0 "
            : "=r"(tmp)
            :
        );
        return (tmp >> 23) & 1;
}

/*
 *  bsp_pretasking_hook
 *
 *  BSP pretasking hook.  Called just before drivers are initialized.
 *  Used to setup libc and install any BSP extensions     .
 */

void bsp_pretasking_hook(void)
{
  bsp_spurious_initialize();
}

/*
 *  bsp_start
 *
 *  This routine does the bulk of the system initialization.
 */
void bsp_start( void )
{
  /* Set Idle thread function */
  Configuration.idle_task = _BSP_Thread_Idle_body;

  CPU_SPARC_HAS_SNOOPING = set_snooping();
}
