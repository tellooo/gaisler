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

#ifndef DRV_MEMSCRUB_H
#define DRV_MEMSCRUB_H

#include <stdint.h>
#include <drv/regs/memscrub.h>
#include <drv/auto.h>
#include <drv/osal.h>

struct memscrub_devcfg;
struct memscrub_priv;

/* return: number of devices registered */
int memscrub_autoinit(void);
/* Register one device */
int memscrub_register(struct memscrub_devcfg *devcfg);
/* Register an array of devices */
int memscrub_init(struct memscrub_devcfg *devcfgs[]);

#define MEMSCRUB_ERR_OK 0
#define MEMSCRUB_ERR_EINVAL -1
#define MEMSCRUB_ERR_ERROR -2

/* 
 * MEMORYSCRUBBER CONFIG register fields
 */
#define CONFIG_DELAY_BIT 8
#define CONFIG_IRQD_BIT 7
#define CONFIG_SERA_BIT 5
#define CONFIG_LOOP_BIT 4
#define CONFIG_MODE_BIT 2
#define CONFIG_ES_BIT 1
#define CONFIG_SCEN_BIT 0

#define CONFIG_DELAY (0xff << CONFIG_DELAY_BIT)
#define CONFIG_IRQD (0x1 << CONFIG_IRQD_BIT)
#define CONFIG_SERA (0x1 << CONFIG_SERA_BIT)
#define CONFIG_LOOP (0x1 << CONFIG_LOOP_BIT)
#define CONFIG_MODE (0x3 << CONFIG_MODE_BIT)
#define CONFIG_ES (0x1 << CONFIG_ES_BIT)
#define CONFIG_SCEN (0x1 << CONFIG_SCEN_BIT)
#define CONFIG_MODE_SCRUB (0x0 << CONFIG_MODE_BIT)
#define CONFIG_MODE_REGEN (0x1 << CONFIG_MODE_BIT)
#define CONFIG_MODE_INIT (0x2 << CONFIG_MODE_BIT)

#define MEMSCRUB_OPTIONS_INTERRUPTDONE_ENABLE CONFIG_IRQD
#define MEMSCRUB_OPTIONS_INTERRUPTDONE_DISABLE 0
#define MEMSCRUB_OPTIONS_EXTERNALSTART_ENABLE CONFIG_ES
#define MEMSCRUB_OPTIONS_EXTERNALSTART_DISABLE 0
#define MEMSCRUB_OPTIONS_LOOPMODE_ENABLE CONFIG_LOOP
#define MEMSCRUB_OPTIONS_LOOPMODE_DISABLE 0
#define MEMSCRUB_OPTIONS_SECONDARY_MEMRANGE_ENABLE CONFIG_SERA
#define MEMSCRUB_OPTIONS_SECONDARY_MEMRANGE_DISABLE 0

int memscrub_dev_count(void);

const struct drv_devreg *memscrub_get_devreg(int dev_no);

struct memscrub_priv *memscrub_open(int dev_no);
int memscrub_close(struct memscrub_priv *priv);

/* Scrubbing modes */
int memscrub_init_start(
        struct memscrub_priv *priv,
        uint32_t value,
        uint8_t delay,
        int options
);
int memscrub_scrub_start(
        struct memscrub_priv *priv,
        uint8_t delay,
        int options
);
int memscrub_regen_start(
        struct memscrub_priv *priv,
        uint8_t delay,
        int options
);
int memscrub_stop(
        struct memscrub_priv *priv
);
int memscrub_active(
        struct memscrub_priv *priv
);

/* Set/get memory ranges */
int memscrub_range_set(
        struct memscrub_priv *priv,
        uint32_t start,
        uint32_t end
);
int memscrub_range_get(
        struct memscrub_priv *priv,
        uint32_t * start,
        uint32_t * end
);
int memscrub_secondary_range_set(
        struct memscrub_priv *priv,
        uint32_t start,
        uint32_t end
);
int memscrub_secondary_range_get(
        struct memscrub_priv *priv,
        uint32_t * start,
        uint32_t * end
);

/* Interrupts */
/* MEMSCRUB Interrupts */
/* Function Interrupt-Code ISR callback prototype.
 * arg	   - Custom arg provided by user
 * access  - AHB Access that failed
 * ahbstatus  - AHB status register of the MEMSCRUB core
 * status  - status register of the MEMSCRUB core
 */
typedef void (*memscrub_isr_t)(
        void *arg,
        uint32_t ahbaccess,
        uint32_t ahbstatus,
        uint32_t scrubstatus
);
int memscrub_isr_register(
        struct memscrub_priv *priv,
        memscrub_isr_t isr,
        void * data
);
int memscrub_isr_unregister(
        struct memscrub_priv *priv
);
int memscrub_error_status(
        struct memscrub_priv *priv,
        uint32_t *ahbaccess,
        uint32_t *ahbstatus,
        uint32_t *scrubstatus
);

/* Set the different error thresholds. */

/* 
 * MEMORYSCRUBBER AHBS register fields
 */
#define AHBS_CECNT_BIT 22
#define AHBS_UECNT_BIT 14
#define AHBS_DONE_BIT 13
#define AHBS_SEC_BIT 11
#define AHBS_SBC_BIT 10
#define AHBS_CE_BIT 9
#define AHBS_NE_BIT 8
#define AHBS_HW_BIT 7
#define AHBS_HM_BIT 3
#define AHBS_HS_BIT 0

#define AHBS_CECNT (0x3ff << AHBS_CECNT_BIT)
#define AHBS_UECNT (0xff << AHBS_UECNT_BIT)
#define AHBS_DONE (1 << AHBS_DONE_BIT)
#define AHBS_SEC (1 << AHBS_SEC_BIT)
#define AHBS_SBC (1 << AHBS_SBC_BIT)
#define AHBS_CE (1 << AHBS_CE_BIT)
#define AHBS_NE (1 << AHBS_NE_BIT)
#define AHBS_HW (1 << AHBS_HW_BIT)
#define AHBS_HM (0xf << AHBS_HM_BIT)
#define AHBS_HS (0x7 << AHBS_HS_BIT)

/* 
 * MEMORYSCRUBBER STAT register fields
 */
#define STAT_RUNCOUNT_BIT 22
#define STAT_BLKCOUNT_BIT 14
#define STAT_DONE_BIT 13
#define STAT_BURSTLEN_BIT 1
#define STAT_ACTIVE_BIT 0

#define STAT_RUNCOUNT (0x3ff << STAT_RUNCOUNT_BIT)
#define STAT_BLKCOUNT (0xff << STAT_BLKCOUNT_BIT)
#define STAT_DONE (0x1 << STAT_DONE_BIT)
#define STAT_BURSTLEN (0xf << STAT_BURSTLEN_BIT)
#define STAT_ACTIVE (0x1 << STAT_ACTIVE_BIT)

/* 
 * MEMORYSCRUBBER AHBERC register fields
 */
#define AHBERC_CECNTT_BIT 22
#define AHBERC_UECNTT_BIT 14
#define AHBERC_CECTE_BIT 1
#define AHBERC_UECTE_BIT 0

#define AHBERC_CECNTT (0x3ff << AHBERC_CECNTT_BIT)
#define AHBERC_UECNTT (0xff << AHBERC_UECNTT_BIT)
#define AHBERC_CECTE (0x1 << AHBERC_CECTE_BIT)
#define AHBERC_UECTE (0x1 << AHBERC_UECTE_BIT)

/* 
 * MEMORYSCRUBBER ETHRES register fields
 */
#define ETHRES_RECT_BIT 22
#define ETHRES_BECT_BIT 14
#define ETHRES_RECTE_BIT 1
#define ETHRES_BECTE_BIT 0

#define ETHRES_RECT (0x3ff << ETHRES_RECT_BIT)
#define ETHRES_BECT (0xff << ETHRES_BECT_BIT)
#define ETHRES_RECTE (0x1 << ETHRES_RECTE_BIT)
#define ETHRES_BECTE (0x1 << ETHRES_BECTE_BIT)

#define MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_ENABLE AHBERC_CECTE
#define MEMSCRUB_OPTIONS_AHBERROR_CORTHRES_DISABLE 0
#define MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_ENABLE AHBERC_UECTE
#define MEMSCRUB_OPTIONS_AHBERROR_UNCORTHRES_DISABLE 0
#define MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_ENABLE ETHRES_RECTE
#define MEMSCRUB_OPTIONS_SCRUBERROR_RUNTHRES_DISABLE 0
#define MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_ENABLE ETHRES_BECTE
#define MEMSCRUB_OPTIONS_SCRUBERROR_BLOCKTHRES_DISABLE 0
int memscrub_ahberror_setup(
        struct memscrub_priv *priv,
        int uethres,
        int cethres,
        int options
);
int memscrub_scruberror_setup(
        struct memscrub_priv *priv,
        int blkthres,
        int runthres,
        int options
);
int memscrub_scrub_position(
        struct memscrub_priv *priv,
        uint32_t * position
);


/** PRIVATE **/
/* Driver private structure. Shall never be referenced by user. */
struct memscrub_priv {
        volatile struct memscrub_regs *regs;
        int interrupt;
        int burstlen;
        int blockmask;
        struct osal_isr_ctx isr_ctx;
        volatile memscrub_isr_t isr;
        volatile void *isr_arg;
        uint8_t open;
};

struct memscrub_devcfg {
        struct drv_devreg regs;
        struct memscrub_priv priv;
};

#endif

