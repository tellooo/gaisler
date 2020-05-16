/*
 * Copyright (c) 2019, Cobham Gaisler AB
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

/* Application example for using the memory controller with EDAC together with
 * the memory scrubber under the Cobham Gaisler BCC environment.
 * 
 *-----------------------------------------------------------------------
 * Operation:
 *
 * This test sets up the memscrub device to repeatedly scrub a
 * memory area. If multiple correctable errors occur during the same
 * scrubber iteration, the scrubber is switched into regeneration
 * mode. When regeneration has completed, the ordinary scrubbing is
 * resumed.
 *
 * The driver can be used in two modes, a "managed" mode where we
 * get an IRQ on every single error on the AHB bus and every completed
 * scrub run, and a silent mode where it sets up the scrubber to loop
 * in the background and only interrupt after multiple correctable
 * errors condition or on an uncorrectable error. The driver is
 * completely interrupt driven, and in the silent mode it does not
 * consume any CPU time during normal scrubbing operation.
 *
 * An software FIFO is used to communicate status information
 * from the driver's interrupt handler to the user. Depending on mode,
 * different amount of messages will be sent. The driver ignores errors 
 * resulting from the FIFO being full, therefore the user is not 
 * required to read out the queue. The application needs to provide 
 * room for the message queue via the configuration table.
 *
 * Configuration:
 *
 * The test is configured via a number of driver resources:
 *   "memstart","memsize":        Determines the memory area to scrub
 *   "opermode":                  Operating mode:
 *                                  0=Quiet mode
 *                                  1=Verbose mode
 *                                  2=Semi-verbose mode (done but no single-CE messages)
 *                                (default: 0)
 *   "regenthres":                Number of errors before regeneration mode
 *                                is entered (default: 5)
 *   "scrubdelay","regendelay":   Delay time in cycles between processing 
 *                                bursts in scrub/regeneration modes.
 *                                (default: 100 scrub / 10 regen)
 *-----------------------------------------------------------------------
 *
 * Author: Javier Jalle/Magnus Hjorth, Martin Åberg, Cobham Gaisler
 * Contact: support@gaisler.com
 */

#include <stdio.h>
#include <stdlib.h>
#include "msgq.h"
#include <bcc/version.h>
#include <drv/osal.h>
#include <drv/nelem.h>
#include <drv/drvret.h>
#include <drv/memscrub.h>
#include <drv/gr716/memscrub.h>

static const char *DESCRIPTION = "Memory scrubber example.\n";

/*
 * GR716 NOTE
 * The bus which memscrub0 probes is configurable in GR716. It is controlled by
 * the "Force Scrubber (FS)" in SYS.CFG.SCFG - Interrupt test configuration
 * register. Enable using:
 *   grmon3> set gpreg2::scfg::fs 1
 *
 * If this bit is 0, then all scrub accesses will get bus error response.  It
 * is possible to set and unset this bit when running to collect uncorrectable
 * erros.
 */

#include <bcc/capability.h>
#ifdef BCC_BSP_gr716
 #define CFG_TARGET_GR716 1
#else
 #define CFG_TARGET_GR716 0
#endif

#define OPERMODE_QUIET 0
#define OPERMODE_VERBOSE 1
#define OPERMODE_SEMIVERBOSE 2

/* -------------- Application setup ----------   */
#if CFG_TARGET_GR716
 /* GR716 local data RAM */
 #ifndef MEMSTART
  #define MEMSTART (0x30000000)
 #endif
 #ifndef MEMSIZE
  #define MEMSIZE  (64 * 1024)
 #endif
#else
 #ifndef MEMSTART
  #define MEMSTART (0x40000000)
 #endif
 #ifndef MEMSIZE
  #define MEMSIZE  (0x01000000)
 #endif
#endif

#ifndef OPERMODE
#define OPERMODE OPERMODE_QUIET
#endif
#ifndef REGEN_DELAY
#define REGEN_DELAY 5
#endif
#ifndef REGEN_THRESHOLD
#define REGEN_THRESHOLD 5
#endif
#ifndef SCRUB_DELAY
#define SCRUB_DELAY 10
#endif

#define MEMEND (MEMSTART + MEMSIZE -1)
int opermode = OPERMODE;

static void user_isr(
        void *arg,
        uint32_t ahbaccess,
        uint32_t ahbstatus,
        uint32_t scrubstatus
);

/* ----------------- Application code ---------------- */

/* /////////
 * Report task. Reads the scrubber message queue and prints a text
 * message whenever something has happened. */

void report_task_entry(void)
{
    int ret;
    struct memscrub_message msg;
    while (1) {
        if (0) {
            bcc_power_down();
        }
        ret = memscrub_msgq_pop(1,&msg);
        if (ret != 0){
            printf("[R] Fail when obtaining message\n");
            continue;
        }
        memscrub_msgq_print(&msg);
    }
}

int testit(struct memscrub_priv *dev)
{
    int options;
    int optionsahb, optionsscrub;
    int ret;

    printf("-- Scrubber BCC " __BCC_VERSION_STRING " test application --\n");  
    printf("Config: Mem range: %08x-%08x\nOpermode:%s\n", 
            (unsigned int)MEMSTART, 
            (unsigned int)(MEMSTART+MEMSIZE-1),
            (opermode == OPERMODE_SEMIVERBOSE)? 
                "semiverbose":
                    ((opermode == OPERMODE_VERBOSE)?
                        "verbose":(
                            (opermode == OPERMODE_QUIET)?
                                "quiet": "unknown"))
            );

    /* Init msgq */
    ret = memscrub_msgq_init();
    if (ret != 0) {
        printf("Error: memscrub msgq init!\n");
        exit(1);
    }

    /* Stop scrubbing */
    ret = memscrub_stop(dev);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub stop!\n");
        exit(1);
    }

    /* Set memory range */
    ret = memscrub_range_set(dev, MEMSTART,MEMEND);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub set range!\n");
        exit(1);
    }

    /* Get options */
    switch(opermode){
        case OPERMODE_QUIET:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_DISABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_ENABLE;
            options = MEMSCRUB_OPTIONS_LOOPMODE_ENABLE;
            break;
        case OPERMODE_VERBOSE:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_ENABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_DISABLE;
            options = MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE;
            break;
        case OPERMODE_SEMIVERBOSE:
        default:
            optionsahb = MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE |
                MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_DISABLE;
            optionsscrub = MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE |
                MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_ENABLE;
            options = MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE;
            break;
    }

    /* Setup scrubber thresholds */
    ret = memscrub_ahberror_setup(dev, 0,0,optionsahb);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub ahberr setup!\n");
        exit(1);
    }
    ret = memscrub_scruberror_setup(dev, 0,REGEN_THRESHOLD, optionsscrub);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub scruberr setup!\n");
        exit(1);
    }

    /* Register and enable ISR */
    ret = memscrub_isr_register(dev, user_isr,dev);  
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub isr register!\n");
        exit(1);
    }


    /* Start scrubber */
    ret = memscrub_scrub_start(dev, SCRUB_DELAY,options);
    if (ret != MEMSCRUB_ERR_OK){
        printf("Error: memscrub start!\n");
        exit(1);
    }

    report_task_entry();
    printf("never\n");
    return 0;
}

int example0(void)
{
        const int DEVNO = 0;
        struct memscrub_priv *dev;

        printf("%s: open memscrub%d -> ", __func__, DEVNO);
        dev = memscrub_open(DEVNO);
        if (!dev) {
                printf("FAILED\n");
                return 1;
        }
        printf("OK\n\n");

        testit(dev);
        memscrub_close(dev);

        return 0;
}

int main(void)
{
        if (CFG_TARGET_GR716) {
                memscrub_init(GR716_MEMSCRUB_DRV_ALL);
        } else {
                memscrub_autoinit();
        }

        puts("");
        puts("");
        puts(DESCRIPTION);

        const int dev_count = memscrub_dev_count();

        printf("INFO: memscrub_dev_count() -> %d\n", dev_count);
        if (dev_count < 1) {
                printf("INFO: no MEMSCRUB device found\n");
                return 1;
        }
        for (int i = 0; i < dev_count; i++) {
                const struct drv_devreg *devreg = NULL;

                devreg = memscrub_get_devreg(i);
                printf(
                        "  memscrub%d: addr=%p, interrupt=%d\n",
                        i,
                        (void *) devreg->addr,
                        devreg->interrupt
                );
        }

        example0();

        return 0;
}

int ce_count;
int ue_count;

/* Interrupt service routine */
static void user_isr(
        void *arg,
        uint32_t ahbaccess,
        uint32_t ahbstatus,
        uint32_t scrubstatus
)
{
    int tmp;
    int runcount;
    int action=0; /* 0=None, 1=Restart scrubbing, 2=Switch to regen*/

    /* Push messages */
    if (ahbstatus & AHBS_DONE){
        memscrub_msgq_push_done(ahbaccess, ahbstatus, scrubstatus);
        action=1;
    }
    if (ahbstatus & AHBS_NE){
        memscrub_msgq_push_error(ahbaccess, ahbstatus, scrubstatus);
    }

    /* Update CE stats */
    if (ahbstatus & AHBS_CE){
        tmp = ((ahbstatus & AHBS_CECNT)>> AHBS_CECNT_BIT);
        ce_count += tmp;
        /* Generate CE threshold flag manually for verbose mode */
        if (opermode == OPERMODE_VERBOSE){
            runcount = (scrubstatus >> STAT_RUNCOUNT_BIT) & STAT_RUNCOUNT;
            if(runcount >= REGEN_THRESHOLD){
                action = 2;
            }
        }
    }

    /* Update UE stats */
    if ((ahbstatus & (AHBS_NE|AHBS_CE|AHBS_SBC|AHBS_SEC))==AHBS_NE){
        tmp = ((ahbstatus & AHBS_UECNT)>> AHBS_UECNT_BIT);
        ue_count += tmp;
    }

    /* Check run count threshold */
    if ((ahbstatus & AHBS_SEC)){
        action=2;
    }

    struct memscrub_priv *priv = arg;
    /* Perform action*/
    if (action == 1){
        /* Restart scrubbing */
        memscrub_scrub_start(priv, SCRUB_DELAY, 
                (opermode == OPERMODE_QUIET)? 
                    MEMSCRUB_OPTIONS_LOOPMODE_ENABLE:
                    MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE);
    }else if (action == 2){
        /* Switch to regen mode */
        memscrub_regen_start(priv, REGEN_DELAY,
                MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE);
        /* Send message */
        memscrub_msgq_push_regen();
    }
}

