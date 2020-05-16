#include "priv.h"

int memscrub_secondary_range_set(
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
	REG_WRITE(&priv->regs->rangel2, start);
	REG_WRITE(&priv->regs->rangeh2, end);

	DBG("MEMSCRUB 2nd range: 0x%08x-0x%08x\n",
			(unsigned int) start,
			(unsigned int) end);

	return MEMSCRUB_ERR_OK;
}

int memscrub_secondary_range_get(
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
	*start = REG_READ(&priv->regs->rangel2);
	*end = REG_READ(&priv->regs->rangeh2);

	return MEMSCRUB_ERR_OK;
}

