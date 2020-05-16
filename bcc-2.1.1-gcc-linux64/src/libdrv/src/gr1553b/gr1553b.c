/*
 * Copyright 2018 Cobham Gaisler AB
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*  GR1553B driver, used by BC, RT and/or BM driver
 *
 * OVERVIEW
 * ========
 *  See header file
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <drv/osal.h>
#include <drv/auto.h>
#include <drv/drvret.h>
#include <drv/gr1553b.h>

/* Driver Manager interface for BC, RT, BM, BRM, BC-BM and RT-BM */

#define GR1553B_WRITE_REG(adr, val) (*(volatile uint32_t *)(adr) = (uint32_t)(val))
#define GR1553B_READ_REG(adr) (*(volatile uint32_t *)(adr))

#define FEAT_BC 0x1
#define FEAT_RT 0x2
#define FEAT_BM 0x4

#define ALLOC_BC 0x1
#define ALLOC_RT 0x2
#define ALLOC_BM 0x4

/* Device lists */
struct gr1553_device_feature *gr1553_bm_root = NULL;
struct gr1553_device_feature *gr1553_rt_root = NULL;
struct gr1553_device_feature *gr1553_bc_root = NULL;

/* Add 'feat' to linked list pointed to by 'root'. A minor is also assigned. */
void gr1553_list_add
	(
	struct gr1553_device_feature **root,
	struct gr1553_device_feature *feat
	)
{
	int minor;
	struct gr1553_device_feature *curr;

	if ( *root == NULL ) {
		*root = feat;
		feat->next = NULL;
		feat->minor = 0;
		return;
	}

	minor = 0;
retry_new_minor:
	curr = *root;
	while ( curr->next ) {
		if ( curr->minor == minor ) {
			minor++;
			goto retry_new_minor;
		}
		curr = curr->next;
	}

	feat->next = NULL;
	feat->minor = minor;
	curr->next = feat;
}

static struct gr1553_device_feature *gr1553_list_find
	(
	struct gr1553_device_feature *root,
	int minor
	)
{
	struct gr1553_device_feature *curr = root;
	while ( curr ) {
		if ( curr->minor == minor ) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

GR1553B gr1553_bc_open(int minor)
{
	struct gr1553_device_feature *feat;

	feat = gr1553_list_find(gr1553_bc_root, minor);
	if ( feat == NULL )
		return NULL;

	/* Only possible to allocate is RT and BC is free,
	 * this is beacuse it is not possible to use the
	 * RT and the BC at the same time.
	 */
	if ( feat->dev->alloc & (ALLOC_BC|ALLOC_RT) )
		return NULL;

	/* Alloc BC device */
	feat->dev->alloc |= ALLOC_BC;

	return feat->dev->dev;
}

void gr1553_bc_close(GR1553B dev)
{
	struct gr1553_device *d;
	if (dev) {
		d = &dev->priv;
		d->alloc &= ~ALLOC_BC;
	}
}

void *gr1553_bc_get(GR1553B dev)
{
	struct gr1553_device *d = &dev->priv;

	return &d->p.bc;
}

GR1553B gr1553_rt_open(int minor)
{
	struct gr1553_device_feature *feat;

	feat = gr1553_list_find(gr1553_rt_root, minor);
	if ( feat == NULL )
		return NULL;

	/* Only possible to allocate is RT and BC is free,
	 * this is beacuse it is not possible to use the
	 * RT and the BC at the same time.
	 */
	if ( feat->dev->alloc & (ALLOC_BC|ALLOC_RT) )
		return NULL;

	/* Alloc RT device */
	feat->dev->alloc |= ALLOC_RT;

	return feat->dev->dev;
}

void gr1553_rt_close(GR1553B dev)
{
	struct gr1553_device *d;
	if (dev) {
		d = &dev->priv;
		d->alloc &= ~ALLOC_RT;
	}
}

void *gr1553_rt_get(GR1553B dev)
{
	struct gr1553_device *d = &dev->priv;

	return &d->p.rt;
}

GR1553B gr1553_bm_open(int minor)
{
	struct gr1553_device_feature *feat;

	feat = gr1553_list_find(gr1553_bm_root, minor);
	if ( feat == NULL )
		return NULL;

	/* Only possible to allocate is RT and BC is free,
	 * this is beacuse it is not possible to use the
	 * RT and the BC at the same time.
	 */
	if ( feat->dev->alloc & ALLOC_BM )
		return NULL;

	/* Alloc BM device */
	feat->dev->alloc |= ALLOC_BM;

	return feat->dev->dev;
}

void gr1553_bm_close(GR1553B dev)
{
	struct gr1553_device *d;
	if (dev) {
		d = &dev->priv;
		d->alloc &= ~ALLOC_BM;
	}
}

void *gr1553_bm_get(GR1553B dev)
{
	struct gr1553_device *d = &dev->priv;

	return &d->bm;
}

static void gr1553_disable_dma(struct gr1553_device *priv)
{
	struct gr1553b_regs *regs;

	if (priv == NULL)
		return;

	regs = (struct gr1553b_regs *)priv->dev->regs.addr;

	/* Stop IRQ */
	GR1553B_WRITE_REG(&regs->imask, 0);
	GR1553B_WRITE_REG(&regs->irq, 0xffffffff);
	/* Stop BC if not already stopped (just in case) */
	GR1553B_WRITE_REG(&regs->bc_ctrl, 0x15520204);
	/* Stop RT rx (just in case) */
	GR1553B_WRITE_REG(&regs->rt_cfg, 0x15530000);
	/* Stop BM logging (just in case) */
	GR1553B_WRITE_REG(&regs->bm_ctrl, 0);
	/* Set codec version. This is only supported by some devices, i.e. GR740.
	 * It will not have any effect on devices that does not support this bit.
	 */
	GR1553B_WRITE_REG(&regs->hwcfg, 1<<12);
}

static int dev_count;
static struct drv_list devlist;

int gr1553_dev_count(void)
{
	return dev_count;
}

int gr1553_init(struct gr1553_devcfg *devcfgs[])
{
        struct gr1553_devcfg **dev = &devcfgs[0];

        while (*dev) {
                gr1553_register(*dev);
                dev++;
        }
        return DRV_OK;
}

/* Register the different functionalities that the
 * core supports.
 */
void gr1553_register(struct gr1553_devcfg *dev)
{
	struct gr1553_device *priv;
	struct gr1553_device_feature *feat;
	struct gr1553b_regs *regs;

        drv_list_addtail(&devlist, &dev->regs.node);
        dev_count++;

	priv = &dev->priv;
	memset(priv, 0, sizeof(struct gr1553_device));
	priv->dev = dev;
	priv->alloc = 0;
	priv->features = 0;

	/* Get device information from AMBA PnP information */
	regs = (struct gr1553b_regs *)dev->regs.addr;

	gr1553_disable_dma(priv);

	if ( GR1553B_READ_REG(&regs->bm_stat) & GR1553B_BM_STAT_BMSUP ) {
		priv->features |= FEAT_BM;
		feat = &priv->feat[0];
		feat->dev = priv;
		/* Init Minor and Next */
		gr1553_list_add(&gr1553_bm_root, feat);
	}

	if ( GR1553B_READ_REG(&regs->bc_stat) & GR1553B_BC_STAT_BCSUP ) {
		priv->features |= FEAT_BC;
		feat = &priv->feat[1];
		feat->dev = priv;
		/* Init Minor and Next */
		gr1553_list_add(&gr1553_bc_root, feat);
	}

	if ( GR1553B_READ_REG(&regs->rt_stat) & GR1553B_RT_STAT_RTSUP ) {
		priv->features |= FEAT_RT;
		feat = &priv->feat[2];
		feat->dev = priv;
		/* Init Minor and Next */
		gr1553_list_add(&gr1553_rt_root, feat);
	}
        return;
}

void gr1553b_unregister(struct gr1553_devcfg *dev)
{
        struct drv_node *prev = NULL;
        struct drv_node *curr = NULL;

        curr = devlist.head;
        while (curr && curr != &dev->regs.node) {
                prev = curr;
                curr = curr->next;
        }
        if (curr) {
                if (curr == devlist.head) {
                        devlist.head = curr->next;
                }
                if (curr == devlist.tail) {
                        devlist.tail = prev;
                }
                if (prev) {
                        prev->next = curr->next;
                }
        }
	gr1553_disable_dma(&dev->priv);
}

void gr1553b_fatal_error(int error)
{
	/* Do nothing */
	(void)error; /* Remove compiler warning (-Wunused-variable) */
}

