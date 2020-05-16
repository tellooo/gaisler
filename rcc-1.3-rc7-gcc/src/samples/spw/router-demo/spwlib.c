/*
 * SpaceWire packet pool allocating library software
 *
 * Copyright (c) 2010 Aeroflex Gaisler AB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "spwlib.h"

#ifndef NULL
#define NULL 0
#endif

#ifdef SILENT
#define PRINTF(x...) 
#else
#define PRINTF(x...) printf(x)
#endif

void spwlib_poolset_print(struct spwlib_poolset *poolset)
{
	int i, totsize;
	struct spwlib_pool *pool;

	totsize = 0;
	/* Print Configuration */
	PRINTF("MAX Data Length:           %d\n", poolset->max_data_len);
	PRINTF("MAX Header Length:         %d\n", poolset->max_hdr_len);
	PRINTF("Number of Pools:           %d\n", poolset->pool_cnt);
	for ( i=0; i<poolset->pool_cnt; i++) {
		pool = &poolset->pools[i];
		PRINTF(" POOL[%d] DATA SIZE:        %d\n", i, pool->cfg.dsize);
		PRINTF(" POOL[%d] HEADER SIZE:      %d\n", i, pool->cfg.hsize);
		PRINTF(" POOL[%d] COUNT:            %d\n", i, pool->cfg.count);
		PRINTF(" POOL[%d] PKTs IN POOL:     %d\n", i, pool->in_pool);
		PRINTF(" POOL[%d] PKTBASE:          %p\n", i, pool->pktbase);
		PRINTF(" POOL[%d] PKTS.HEAD:        %p\n", i, pool->pkts.head);
		PRINTF(" POOL[%d] PKTS.TAIL:        %p\n", i, pool->pkts.tail);
		totsize += pool->cfg.count * (pool->cfg.dsize + pool->cfg.hsize);
	}
	PRINTF("Total size of DATA&HDR:    %d Bytes\n", totsize);
}

struct spwlib_poolset *spwlib_poolset_alloc(struct spwlib_pool_cfg *cfg)
{
	struct spwlib_poolset *poolset;
	struct spwlib_pool *pool;
	struct spwlib_pool_cfg *desc;
	struct spwlib_pkt *pktbase;
	char *buf;
	int i,pool_cnt,pkt_cnt,max_data_size,max_hdr_size,totsize;

	/* Get memory for all pool descriptions */
	desc = cfg;
	pool_cnt = 0;
	pkt_cnt=0;
	max_data_size=0;
	max_hdr_size=0;
	while((desc->dsize || desc->hsize) && desc->count){
		pool_cnt++;
		pkt_cnt += desc->count;

		if ( desc->dsize > max_data_size ){
			max_data_size = desc->dsize;
		}
		if ( desc->hsize > max_hdr_size ){
			max_hdr_size = desc->hsize;
		}
		desc++;
	}

	totsize = sizeof(struct spwlib_pool) +
	          (pool_cnt * sizeof(struct spwlib_pool)) +
	          (pkt_cnt * sizeof(struct spwlib_pkt));
	buf = malloc( totsize );
	if ( !buf ) {
		return NULL;
	}
	memset(buf, 0, totsize);

	/* Init global Pool paramters */
	poolset = (struct spwlib_poolset *)buf;
	poolset->max_data_len = max_data_size;
	poolset->max_hdr_len = max_hdr_size;
	poolset->pool_cnt = pool_cnt;
	poolset->pools = (struct spwlib_pool *)(poolset+1);

	/* Init all Pool's read-only paramters */
	desc = cfg;
	pool = &poolset->pools[0];
	/* Make packet start after the packet pool structs */
	pktbase = (struct spwlib_pkt *)
	      ((unsigned int)pool + pool_cnt*sizeof(struct spwlib_pool));
	for (i=0; i<poolset->pool_cnt; i++) {
		pool->cfg = *desc;
		pool->index = i;
		pool->pktbase = pktbase;

		/* reset non-read-only paramters */
		pool->in_pool = 0;
		pool->pkts.head = pool->pkts.tail = NULL;

		pktbase += pool->cfg.count;
		desc++;
		pool++;
	}

	return poolset;
}

void spwlib_poolset_free(struct spwlib_poolset *poolset)
{
	if ( poolset->pools ) {
		free(poolset->pools);
		poolset->pools = NULL;
	}
	poolset->max_data_len = 0;
	poolset->max_hdr_len = 0;
	poolset->pool_cnt = 0;
}

static int spwlib_pool_init(
	struct spwlib_pool *pool,
	struct spwlib_alloc_cfg *memalgo,
	int alloc
	)
{
	struct spwlib_pool_cfg *cfg = &pool->cfg;
	int cnt = cfg->count;
	struct spwlib_pkt *pkt;

	pkt = pool->pktbase;
	pool->in_pool = cnt;
	
	if ( cnt > 0 ){
		pool->pkts.head = pkt;
		do {
			pkt->dlen = 0;
			pkt->hlen = 0;
			pkt->flags = 0;

			if ( alloc ) {
				pkt->data = memalgo->alloc(
							memalgo->data,
							pool->index,
							0,
							cfg->dsize);
				pkt->hdr = memalgo->alloc(
							memalgo->data,
							pool->index,
							1,
							cfg->hsize);
				if ( !pkt->data || !pkt->hdr )
					return -1; /* Failed to allocate memory */
			}
			pkt->next = pkt+1;
			pkt->pool = pool;
			pkt++;
			cnt--;
		} while ( cnt > 0 );
		/* Mark end of List */
		(pkt-1)->next = NULL;
		pool->pkts.tail = (pkt-1);
	} else {
		pool->pkts.head = pool->pkts.tail = NULL;
	}

	return 0;
}

/* Reset packet structure fields, and reinitialize pool's packet list */
int spwlib_pool_reinit(struct spwlib_pool *pool)
{
	return spwlib_pool_init(pool, NULL, 0);
}

/* Setup every Pool in the Poolset structure:
 *   - Pool Packet Lists filled with packet
 *   - All Packets in all Pools are initialized
 *   - Packets are assigned a DATA and HEADER pointer.
 */
int spwlib_poolset_pkt_alloc(
	struct spwlib_poolset *poolset,
	struct spwlib_alloc_cfg *memalgo
	)
{
	struct spwlib_pool *pool;
	int i;

	if ( !poolset->pools || !memalgo || !memalgo->reset || !memalgo->alloc )
		return -1;

	/* Reset Memory allocation algorithm */
	memalgo->reset(memalgo->data);

	/* Init pool one pool at a time */
	pool = poolset->pools;
	for (i=0; i<poolset->pool_cnt; i++) {
		if ( spwlib_pool_init(pool, memalgo, 1) ) {
			return -1;
		}

		pool++;
	}

	return 0;
}

int spwlib_poolset_reinit(struct spwlib_poolset *poolset)
{
	struct spwlib_pool *pool;
	int i;

	if ( !poolset->pools )
		return -1;

	/* Init pool one pool at a time */
	pool = poolset->pools;
	for (i=0; i<poolset->pool_cnt; i++) {
		if ( spwlib_pool_reinit(pool) ) {
			return -1;
		}

		pool++;
	}

	return 0;
}

struct spwlib_pkt *spwlib_pkt_take(
	struct spwlib_poolset *poolset,
	int data_size,
	int header_size
	)
{
	struct spwlib_pkt *pkt;
	struct spwlib_pool *pool;
	int i;

	if ( data_size > poolset->max_data_len ){
		/* Requirements does not match even the greatest pool */
		return NULL;
	}

	if ( header_size > poolset->max_hdr_len ){
		/* Requirements does not match even the greatest pool */
		return NULL;
	}

	/* Take first packet that has at least size 'size' bytes in packet */
	pool = poolset->pools;
	for (i=0; i<poolset->pool_cnt; i++) {
		/* Does Packet match this pool's HDR & DATA Lenghts?
		 * And are there free packets available?
		 */
		if ( (pool->cfg.dsize >= data_size) && (pool->cfg.hsize >= header_size) ){
			pkt = pool->pkts.head;
			if ( pkt ) {
				if ( pool->pkts.tail == pkt ) {
					pool->pkts.head = NULL;
					pool->pkts.tail = NULL;
				} else {
					pool->pkts.head = pkt->next;
				}
				pool->in_pool--;
				return pkt;
			}
			/* Try next pool */
		}
		pool++;
	}

	/* Didn't find any packet matching the required size in any pool */
	return NULL;
}

/* Return Packet to Pool to head of packet pool */
void spwlib_pkt_return(struct spwlib_pkt *pkt)
{
	struct spwlib_pool *pool = pkt->pool;

	pool->in_pool++;
	pkt->next = pool->pkts.head;
	if ( pool->pkts.head == NULL ) {
		pool->pkts.tail = pkt;
	}
	pool->pkts.head = pkt;
}

int spwlib_pkt_chain_take(
	struct spwlib_poolset *poolset,
	int data_size,
	int header_size,
	int max,
	struct spwlib_list *list
	)
{
	struct spwlib_pkt *pkt, *start, *end;
	int i, pkts_left;
	struct spwlib_pool *pool;

	if ( data_size > poolset->max_data_len ){
		/* Will not fit any packet */
		return 0;
	}

	if ( header_size > poolset->max_hdr_len ){
		/* Will not fit any packet */
		return 0;
	}

	if ( max < 1 )
		return 0;

	/* Take first packet that has at least size 'size' bytes in packet */
	pkts_left = max;
	pool = poolset->pools;
	for (i=0; i<poolset->pool_cnt; i++) {
		if ( (pool->cfg.dsize >= data_size) && (pool->cfg.hsize >= header_size) ){
			pkt = pool->pkts.head;
			if ( pkt ){
				start = pkt;
				do {
					pkts_left--;
					end = pkt;
				} while( (pkts_left>0) && (pkt!=pool->pkts.tail) && (pkt=pkt->next));

				/* Add packets to requested chain */
				if ( list->head == NULL ){
					list->head = start;
					list->tail = end;
				}else{
					list->tail->next = start;
					list->tail = end;
				}

				/* Remove packets from pool */
				if ( pool->pkts.tail == end ){
					pool->pkts.head = NULL;
					pool->pkts.tail = NULL;
				}else{
					pool->pkts.head = end->next;
				}
				end->next = NULL;

				/* Return if requested number of packets found */
				if ( pkts_left < 1) {
					list->count += max;
					return max;
				}
			}
			/* Try next pool */
		}
		pool++;
	}

	/* Didn't find enough packets matching the required size */
	list->count += max-pkts_left;
	return (max-pkts_left);
}


/* Take A number of entries from head of list 'list' and put the entires
 * to rlist (result list).
 */
int spwlib_list_take_head_list(struct spwlib_list *list,
				struct spwlib_list *rlist, int max)
{
	int cnt;
	struct spwlib_pkt *pkt, *last;

	pkt = list->head;

	if ( (max < 1) || (pkt == NULL) ) {
		spwlib_list_clr(rlist);
		return 0;
	}
	if (max >= list->count) {
		*rlist = *list;
		cnt = list->count;
		spwlib_list_clr(list);
		return cnt;
	}

	cnt = 0;
	rlist->head = pkt;
	last = pkt;
	while ((cnt < max) && pkt) {
		last = pkt;
		pkt = pkt->next;
		cnt++;
	}
	rlist->tail = last;
	rlist->count = cnt;
	spwlib_list_remove_head_list(list, rlist);
	rlist->tail->next = NULL;
	return cnt;
}

int spwlib_list_count(struct spwlib_list *list)
{
	struct spwlib_pkt *pkt = list->head;
	int cnt = 0;
	while ( pkt ) {
		cnt++;
		pkt = pkt->next;
	}
	return cnt;
}
