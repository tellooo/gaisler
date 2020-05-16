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

#include "priv.h"

static int dev_count;
static struct drv_list devlist = { NULL, NULL };

int memscrub_register(struct memscrub_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int memscrub_init(struct memscrub_devcfg *devcfgs[])
{
        struct memscrub_devcfg **dev = &devcfgs[0];

        while (*dev) {
                memscrub_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int memscrub_dev_count(void)
{
        return dev_count;
}

const struct drv_devreg *memscrub_get_devreg(int dev_no)
{
        const struct
            memscrub_devcfg
            *dev =
            (const struct memscrub_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}


/* IMPLEMENTATION */

static int memscrub_openinit2(struct memscrub_priv *priv);
static int memscrub_openinit(struct memscrub_priv *priv);

struct memscrub_priv *memscrub_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct memscrub_devcfg *dev =
            (struct memscrub_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct memscrub_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }

        priv->isr = NULL;
        priv->interrupt = dev->regs.interrupt;
        priv->regs = (struct memscrub_regs *) dev->regs.addr;

        memscrub_openinit2(priv);

        return priv;
}

int memscrub_close(struct memscrub_priv *priv)
{
        priv->open = 0;
        return MEMSCRUB_ERR_OK;
}

static int memscrub_openinit(struct memscrub_priv *priv)
{
	unsigned int tmp;
	int i,j;

	DBG("MEMSCRUB regs 0x%08x\n", (unsigned int) priv->regs);

	/* Find MEMSCRUB capabilities */
	tmp = REG_READ(&priv->regs->status);
	i = (tmp & STAT_BURSTLEN) >> STAT_BURSTLEN_BIT;
	for (j=1; i>0; i--) j <<= 1;
	priv->burstlen = j;


	/* If scrubber is active, we cannot stop it to read blockmask value */
	if (tmp & STAT_ACTIVE){
		priv->blockmask = 0;
	}else{
		/* Detect block size in bytes and burst length */
		tmp = REG_READ(&priv->regs->rangeh);
		REG_WRITE(&priv->regs->rangeh, 0);
		priv->blockmask = REG_READ(&priv->regs->rangeh);
		REG_WRITE(&priv->regs->rangeh, tmp);
	}

	/* DEBUG print */
	DBG("MEMSCRUB with following capabilities:\n");
	DBG(" -Burstlength: %d AHB bus cycles\n", priv->burstlen);

	return MEMSCRUB_ERR_OK;
}

static int memscrub_openinit2(
	struct memscrub_priv *priv
)
{
	/* Initilize driver struct */
	memscrub_openinit(priv);

	/* Startup Action:
	 *	- Clear status
	 *	- Register ISR
	 */

	/* Initialize hardware by clearing its status */
	REG_WRITE(&priv->regs->ahbstatus, 0);
	REG_WRITE(&priv->regs->status, 0);

	return MEMSCRUB_ERR_OK;
}


int memscrub_init_start(
	struct memscrub_priv *priv,
	uint32_t value,
	uint8_t delay,
	int options
)
{
	uint32_t sts, tmp;
	int i;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Check if scrubber is active */
	sts = REG_READ(&priv->regs->status);
	if (sts & STAT_ACTIVE){
		DBG("MEMSCRUB running.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Check if we need to probe blockmask */
	if (priv->blockmask == 0){
		/* Detect block size in bytes and burst length */
		tmp = REG_READ(&priv->regs->rangeh);
		REG_WRITE(&priv->regs->rangeh, 0);
		priv->blockmask = REG_READ(&priv->regs->rangeh);
		REG_WRITE(&priv->regs->rangeh, tmp);
	}

	/* Set data value */
	for (i=0; i<priv->blockmask; i+=4){
		REG_WRITE(&priv->regs->init,value);
	}

	/* Clear unused bits */
	options = options & ~(CONFIG_MODE | CONFIG_DELAY);

	/* Enable scrubber */
	REG_WRITE(&priv->regs->config, options | 
			((delay << CONFIG_DELAY_BIT) & CONFIG_DELAY) | 
			CONFIG_MODE_INIT | CONFIG_SCEN);

	DBG("MEMSCRUB INIT STARTED\n");

	return MEMSCRUB_ERR_OK;
}

int memscrub_scrub_start(
	struct memscrub_priv *priv,
	uint8_t delay,
	int options
)
{
	uint32_t ctrl,sts;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Check if scrubber is active */
	sts = REG_READ(&priv->regs->status);
	if (sts & STAT_ACTIVE){
		/* Check if mode is not init */
		ctrl = REG_READ(&priv->regs->config);
		if ((ctrl & CONFIG_MODE)==CONFIG_MODE_INIT){
			DBG("MEMSCRUB init running.\n");
			return MEMSCRUB_ERR_ERROR;
		}
	}

	/* Clear unused bits */
	options = options & ~(CONFIG_MODE | CONFIG_DELAY);

	/* Enable scrubber */
	REG_WRITE(&priv->regs->config, options | 
			((delay << CONFIG_DELAY_BIT) & CONFIG_DELAY) | 
			CONFIG_MODE_SCRUB | CONFIG_SCEN);

	DBG("MEMSCRUB SCRUB STARTED\n");

	return MEMSCRUB_ERR_OK;
}

int memscrub_regen_start(
	struct memscrub_priv *priv,
	uint8_t delay,
	int options
)
{
	uint32_t ctrl,sts;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Check if scrubber is active */
	sts = REG_READ(&priv->regs->status);
	if (sts & STAT_ACTIVE){
		/* Check if mode is not init */
		ctrl = REG_READ(&priv->regs->config);
		if ((ctrl & CONFIG_MODE)==CONFIG_MODE_INIT){
			DBG("MEMSCRUB init running.\n");
			return MEMSCRUB_ERR_ERROR;
		}
	}

	/* Clear unused bits */
	options = options & ~(CONFIG_MODE | CONFIG_DELAY);

	/* Enable scrubber */
	REG_WRITE(&priv->regs->config, options | 
			((delay << CONFIG_DELAY_BIT) & CONFIG_DELAY) | 
			CONFIG_MODE_REGEN | CONFIG_SCEN);

	DBG("MEMSCRUB REGEN STARTED\n");

	return MEMSCRUB_ERR_OK;
}

int memscrub_stop(
	struct memscrub_priv *priv
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Disable scrubber */
	REG_WRITE(&priv->regs->config, 0);

	/* Wait until finished */
	while(REG_READ(&priv->regs->status) & STAT_ACTIVE){};

	DBG("MEMSCRUB STOPPED\n");

	return MEMSCRUB_ERR_OK;
}

int memscrub_range_set(
	struct memscrub_priv *priv,
	uint32_t start,
	uint32_t end
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if (end <= start){
		DBG("MEMSCRUB wrong address.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	/* Check if scrubber is active */
	if (REG_READ(&priv->regs->status) & STAT_ACTIVE){
		DBG("MEMSCRUB running.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Set range */
	REG_WRITE(&priv->regs->rangel, start);
	REG_WRITE(&priv->regs->rangeh, end);

	DBG("MEMSCRUB range: 0x%08x-0x%08x\n",
			(unsigned int) start,
			(unsigned int) end);

	return MEMSCRUB_ERR_OK;
}

int memscrub_range_get(
	struct memscrub_priv *priv,
	uint32_t * start,
	uint32_t * end
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if ((start==NULL) || (end == NULL)){
		DBG("MEMSCRUB wrong pointer.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	/* Get range */
	*start = REG_READ(&priv->regs->rangel);
	*end = REG_READ(&priv->regs->rangeh);

	return MEMSCRUB_ERR_OK;
}

int memscrub_ahberror_setup(
	struct memscrub_priv *priv,
	int uethres,
	int cethres,
	int options
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Set AHBERR */
	REG_WRITE(&priv->regs->ahberc, 
			((cethres << AHBERC_CECNTT_BIT) & AHBERC_CECNTT) |
			((uethres << AHBERC_UECNTT_BIT) & AHBERC_UECNTT) |
			(options & (AHBERC_CECTE | AHBERC_UECTE)));

	DBG("MEMSCRUB ahb err: UE[%d]:%s, CE[%d]:%s\n",
			(unsigned int) uethres,
			(options & AHBERC_UECTE)? "enabled":"disabled",
			(unsigned int) cethres,
			(options & AHBERC_CECTE)? "enabled":"disabled"
			);

	return MEMSCRUB_ERR_OK;
}

int memscrub_scruberror_setup(
	struct memscrub_priv *priv,
	int blkthres,
	int runthres,
	int options
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	/* Set ETHRES */
	REG_WRITE(&priv->regs->ethres, 
			((blkthres << ETHRES_BECT_BIT) & ETHRES_BECT) |
			((runthres << ETHRES_RECT_BIT) & ETHRES_RECT) |
			(options & (ETHRES_RECTE | ETHRES_BECTE)));

	DBG("MEMSCRUB scrub err: BLK[%d]:%s, RUN[%d]:%s\n",
			(unsigned int) blkthres,
			(options & ETHRES_BECTE)? "enabled":"disabled",
			(unsigned int) runthres,
			(options & ETHRES_RECTE)? "enabled":"disabled"
			);

	return MEMSCRUB_ERR_OK;
}

int memscrub_scrub_position(
	struct memscrub_priv *priv,
	uint32_t * position
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if (position==NULL){
		DBG("MEMSCRUB wrong pointer.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	*position = REG_READ(&priv->regs->pos);

	return MEMSCRUB_ERR_OK;
}

int memscrub_error_status(
	struct memscrub_priv *priv,
	uint32_t *ahbaccess,
	uint32_t *ahbstatus, 
	uint32_t *scrubstatus
)
{
	uint32_t mask, ahbstatus_val;

	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	if ((ahbaccess==NULL) || (ahbstatus==NULL) || (scrubstatus == NULL)){
		DBG("MEMSCRUB wrong pointer.\n");
		return MEMSCRUB_ERR_EINVAL;
	}

	/* Get hardware status */
	*ahbaccess = REG_READ(&priv->regs->ahbfailing);
	*ahbstatus = ahbstatus_val = REG_READ(&priv->regs->ahbstatus);
	*scrubstatus = REG_READ(&priv->regs->status);

	/* Clear error status */
	mask = 0;
	/* Clear CECNT only if we crossed the CE threshold*/
	if ((ahbstatus_val & AHBS_CE) == 0){
		/* Don't clear the CECNT */
		mask |= AHBS_CECNT;
	}
	/* Clear UECNT only if we crossed the UE threshold*/
	if ((ahbstatus_val & (AHBS_NE|AHBS_CE|AHBS_SBC|AHBS_SEC)) != AHBS_NE){
		/* Don't clear the UECNT */
		mask |= AHBS_UECNT;
	}
	REG_WRITE(&priv->regs->ahbstatus, ahbstatus_val & mask);
	REG_WRITE(&priv->regs->status,0);

	return MEMSCRUB_ERR_OK;
}

int memscrub_active(
	struct memscrub_priv *priv
)
{
	if (priv==NULL){
		DBG("MEMSCRUB not init.\n");
		return MEMSCRUB_ERR_ERROR;
	}

	return REG_READ(&priv->regs->status) & STAT_ACTIVE;
}

