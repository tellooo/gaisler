/*
 * Copyright (c) 2018, Cobham Gaisler AB
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

#define SPICTRL_EVENT_CLEAR ( \
        SPICTRL_EVENT_LT | \
        SPICTRL_EVENT_OV | \
        SPICTRL_EVENT_UN | \
        SPICTRL_EVENT_MME \
)

#define SPICTRL_MODE_SPIMODE_BIT        SPICTRL_MODE_CPHA_BIT
#define SPICTRL_MODE_SPIMODE            (0x3 << SPICTRL_MODE_CPHA_BIT)

static int dev_count;

static struct drv_list devlist = { NULL, NULL };

struct wordleninfo {
        uint8_t len;
        int8_t rxshift;
};

/*
 * Defines for each value in "enum spi_wordlen", a pair of MODE.LEN and SHIFT
 * value for the word length.
 */
static const struct wordleninfo LENTABLE[SPI_WORDLEN_NUM] = {
        /* SPI_WORDLEN_4 */
        {.len = 0x3, .rxshift = 12},
        {.len = 0x4, .rxshift = 11},
        {.len = 0x5, .rxshift = 10},
        {.len = 0x6, .rxshift = 9},
        {.len = 0x7, .rxshift = 8},
        {.len = 0x8, .rxshift = 7},
        {.len = 0x9, .rxshift = 6},
        {.len = 0xa, .rxshift = 5},
        {.len = 0xb, .rxshift = 4},
        {.len = 0xc, .rxshift = 3},
        {.len = 0xd, .rxshift = 2},
        {.len = 0xe, .rxshift = 1},
        /* SPI_WORDLEN_16 */
        {.len = 0xf, .rxshift = 0},
        /* SPI_WORDLEN_32 */
        {.len = 0x0, .rxshift = 0}
};

int spi_register(struct spi_devcfg *devcfg)
{
        drv_list_addtail(&devlist, &devcfg->regs.node);
        devcfg->priv.open = 0;
        dev_count++;
        return DRV_OK;
}

int spi_init(struct spi_devcfg *devcfgs[])
{
        struct spi_devcfg **dev = &devcfgs[0];

        while (*dev) {
                spi_register(*dev);
                dev++;
        }
        return DRV_OK;
}

int spi_dev_count(void)
{
        return dev_count;
}

struct spi_priv *spi_open(int dev_no)
{
        if (dev_no < 0) {
                return NULL;
        }
        if (dev_count <= dev_no) {
                return NULL;
        }

        struct spi_devcfg *dev =
            (struct spi_devcfg *) drv_list_getbyindex(&devlist, dev_no);
        struct spi_priv *priv = &dev->priv;

        uint8_t popen;

        popen = osal_ldstub(&priv->open);
        if (popen) {
                return NULL;
        }
        priv->apbfreq = osal_busfreq();
        priv->regs = (struct spictrl_regs *)dev->regs.addr;
        {
                uint32_t cap;

                cap = priv->regs->capability;
                priv->ssen = !!(cap & SPICTRL_CAPABILITY_SSEN);
                priv->asela = !!(cap & SPICTRL_CAPABILITY_ASELA) && priv->ssen;
                priv->sssz = (cap & SPICTRL_CAPABILITY_SSSZ) >>
                    SPICTRL_CAPABILITY_SSSZ_BIT;
        }

        /* Mask all Interrupts. */
        priv->regs->mask = 0;
        /* Disable core and select master mode. */
        priv->regs->mode = SPICTRL_MODE_MS;
        /* Clear all events. */
        priv->regs->event = SPICTRL_EVENT_CLEAR;
        /* LST bit is not used by driver so force it to zero. */
        priv->regs->command = 0;
        SPIN_INIT(&priv->devlock, "drvspi");

        return priv;
}

int spi_close(struct spi_priv *priv)
{
        /* Mask all Interrupts */
        priv->regs->mask = 0;
        /* Disable SPICTTRL, Select Master mode */
        priv->regs->mode = SPICTRL_MODE_MS;

        priv->open = 0;

        return DRV_OK;
}

uint32_t spi_get_event(struct spi_priv *priv)
{
        return priv->regs->event;
}

void spi_clear_event(struct spi_priv *priv, uint32_t event)
{
        priv->regs->event = event;
}

/* Determine if freq is a valid SPI clock frequency. */
static int validate_freq(unsigned int freq, unsigned int apbfreq)
{
        unsigned int lowest_freq_possible;

        /* Lowest possible when DIV16 is set and PM is 0xf */
        lowest_freq_possible = apbfreq / (16 * 4 * (0xf + 1));

        if (freq < lowest_freq_possible) {
                return DRV_FAIL;
        }
        return DRV_OK;
}

/*
 * Calculate mode word for as high frequency of SCK as possible but not higher
 * than requested frequency (freq).
 */
static uint32_t get_clkmagic(unsigned int freq, unsigned int apbfreq)
{
        uint32_t magic;
        uint32_t div;
        uint32_t div16;
        uint32_t pm;
        uint32_t fact;

        div = ((apbfreq / 2) + (freq - 1)) / freq;

        /* Is DIV16 neccessary? */
        if (16 < div) {
                div = (div + (16 - 1)) / 16;
                div16 = 1;
        } else {
                div16 = 0;
        }

        if (0xf < div) {
                /* FACT adds a factor /2 */
                fact = 0;
                div = (div + (2 - 1)) / 2;
        } else {
                fact = 1;
        }

        pm = div - 1;

        magic =
            (pm << SPICTRL_MODE_PM_BIT) |
            (div16 << SPICTRL_MODE_DIV16_BIT) | (fact << SPICTRL_MODE_FACT_BIT);

        return magic;
}

const struct spi_config SPI_CONFIG_DEFAULT = {
        .freq           = 1000 * 1000,
        .mode           = 0,
        .wordlen        = SPI_WORDLEN_32,
        .intmask        = 0,
        .msb_first      = 0,
        .sync           = 0,
        .aslave         = 0,
        .clock_gap      = 0,
        .tac            = 0,
        .aseldel        = 0,
        .igsel          = 0,
};

int spi_config(struct spi_priv *priv, struct spi_config *cfg)
{
        volatile struct spictrl_regs *regs = priv->regs;
        SPIN_IRQFLAGS(plev);
        uint32_t mode;
        int ret = DRV_OK;

        /* Validate wordlen */
        if (SPI_WORDLEN_NUM <= cfg->wordlen) {
                return DRV_FAIL;
        }
        /* Validate frequency */
        if (DRV_OK != validate_freq(cfg->freq, priv->apbfreq)) {
                return DRV_FAIL;
        }

        mode = SPICTRL_MODE_MS;

        /* Parse SPI mode */
        mode |= (cfg->mode << SPICTRL_MODE_SPIMODE_BIT) & SPICTRL_MODE_SPIMODE;

        /* Parse wordlen and set bits per word */
        mode |= LENTABLE[cfg->wordlen].len << SPICTRL_MODE_LEN_BIT;

        /* Parse frequency */
        mode |= get_clkmagic(cfg->freq, priv->apbfreq);

        mode |=
            ((cfg->msb_first!=0) << SPICTRL_MODE_REV_BIT) & SPICTRL_MODE_REV;

        if (priv->asela) {
                if (cfg->aslave) {
                        /* Slave select is active-low */
                        priv->regs->aslvsel = ~cfg->aslave;
                        mode |= SPICTRL_MODE_ASEL;
                        mode |= (cfg->tac << SPICTRL_MODE_TAC_BIT) &
                            SPICTRL_MODE_TAC;
                        mode |= (cfg->aseldel << SPICTRL_MODE_ASELDEL_BIT) &
                            SPICTRL_MODE_ASELDEL;
                }
        }

        mode |= (cfg->clock_gap << SPICTRL_MODE_CG_BIT) & SPICTRL_MODE_CG;
        mode |= (cfg->igsel << SPICTRL_MODE_IGSEL_BIT) & SPICTRL_MODE_IGSEL;

        SPIN_LOCK_IRQ(&priv->devlock, plev);
        if (regs->mode & SPICTRL_MODE_EN) {
                ret = DRV_STARTED;
                goto out;
        }

        if (cfg->wordlen == SPI_WORDLEN_32) {
                priv->bytelen = 0;
        } else {
                priv->bytelen = (LENTABLE[cfg->wordlen].len+1+7) >> 3;
        }

        if (cfg->msb_first && cfg->wordlen != SPI_WORDLEN_32) {
                priv->rxshift = 16;
                priv->txshift = 16+LENTABLE[cfg->wordlen].rxshift;
        } else {
                priv->rxshift = LENTABLE[cfg->wordlen].rxshift;
                priv->txshift = 0;
        }
        regs->mode = mode;
        regs->mask = cfg->intmask;
        priv->sync = cfg->sync;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return ret;
}

int spi_start(struct spi_priv *priv)
{
        SPIN_IRQFLAGS(plev);
        uint32_t mode;
        int ret = DRV_OK;

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        mode = priv->regs->mode;
        if (mode & SPICTRL_MODE_EN) {
                ret = DRV_STARTED;
                goto out;
        }

        priv->rxfree = ((priv->regs->capability & SPICTRL_CAPABILITY_FDEPTH) >>
                SPICTRL_CAPABILITY_FDEPTH_BIT) + 1;
        priv->regs->event = SPICTRL_EVENT_CLEAR;
        priv->regs->mode |= SPICTRL_MODE_EN;

out:
        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return ret;
}

int spi_stop(struct spi_priv *priv)
{
        SPIN_IRQFLAGS(plev);
        uint32_t mode;

        SPIN_LOCK_IRQ(&priv->devlock, plev);

        mode = priv->regs->mode;
        priv->regs->mode = mode & ~SPICTRL_MODE_EN;

        SPIN_UNLOCK_IRQ(&priv->devlock, plev);
        return DRV_OK;
}

int spi_slave_select(struct spi_priv *priv, uint32_t mask)
{
        if (!priv->ssen || (mask & ~((1<<priv->sssz)-1))) {
                return DRV_NOIMPL;
        }
        if (priv->regs->event & SPICTRL_EVENT_TIP) {
                return DRV_WOULDBLOCK;
        }

        /* Slave select is active-low */
        priv->regs->slvsel = ~mask;
        return DRV_OK;
}

const struct drv_devreg *spi_get_devreg(int dev_no)
{
        const struct
            spi_devcfg
            *dev =
            (const struct spi_devcfg *)
            drv_list_getbyindex(&devlist, dev_no);

        return &dev->regs;
}

