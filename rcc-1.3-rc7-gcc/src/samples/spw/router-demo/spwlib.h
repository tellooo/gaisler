/* SpaceWire Packet Help Library, intended for the GRSPW driver 
 *
 * Packets of the same Header&Data size are arranged in a POOL. Each POOL
 * has a single-linked list holding all free packets. A set of POOLs create a
 * Packet POOLSET.
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

#ifndef __SPWLIB_H__
#define __SPWLIB_H__

/*** Configure SpWLib ***/

/* Define SPWLIB_PKT_EXTRA_DATA to add extra "4byte blocks" per-packet data
 * variables.
 */
/*#define SPWLIB_PKT_EXTRA_DATA 1*/

struct spwlib_pool;

/* SpaceWire Packet (same layout as grspw_pkt with additional fields) */
struct spwlib_pkt {
	struct spwlib_pkt *next;	/* next packet in chain */
	int pkt_id;

	/* Options and status */
	unsigned short flags;
	unsigned char resv;
	unsigned char hlen;/* if hlen is zero hdr is not used */

	/* Data */
	int dlen;
	void *data;
	/* Header */
	void *hdr;

	/* Packet Belongs to this Pool */
	struct spwlib_pool *pool;

	/* Let user */
#ifdef SPWLIB_PKT_EXTRA_DATA
	unsigned int user_data[SPWLIB_PKT_EXTRA_DATA];
#endif
};

/* Single-linked List structure of SpW packets */
struct spwlib_list {
	struct spwlib_pkt *head;
	struct spwlib_pkt *tail;
	int count;
};

/***********  POOLS **********
 *
 *
 * 
 *
 *
 */

/* Packet pool Configuration Description */
struct spwlib_pool_cfg {
	unsigned int dsize;	/*!< Data Length of packet in the pool */
	unsigned int hsize;	/*!< Header Length of packet in the pool */
	unsigned int count;	/*!< Number of packets in the pool */
};
#define SPWLIB_POOL_CFG(dsize, hsize, count) {dsize, hsize, count}
#define SPWLIB_POOL_CFG_END SPWLIB_POOL_CFG(0,0,0)

/* Packet Pool */
struct spwlib_pool {
	struct spwlib_pool_cfg cfg;	/*!< Pool Configuration */
	int index;			/*!< Pool number in array of pools */
	struct spwlib_pkt *pktbase;	/*!< First packet buffer in pool */
	int in_pool;			/*!< Number of Packets Currently in Pool */
	struct spwlib_list pkts;	/*!< Linked List of packets */
};

/* Array of Packet Pools, normally Pools with different packet size.
 * Pools with the same size may be used to increase performance by for
 * example using on-chip memory first, when on-chip pool is empty the
 * SDRAM pool is used.
 */
struct spwlib_poolset {
	int max_data_len;		/*!< Pool's Max Data Size Packet */
	int max_hdr_len;		/*!< Pool's Max Header Size Packet */
	int pool_cnt;			/*!< Number of Packet Pools */
	struct spwlib_pool *pools;	/*!< The Pools, in an array */
};

/* Packet Buffer Memory Allocation Configuration. Functions are used when
 * dynamically allocating memory to the Packets.
 *
 * data    : Custom Data passed as an argument to alloc() and reset()
 * alloc() : Allocate Packet Buffer Memory 
 * reset() : Reset packet Buffer allocation algorithm, reuse memory so 
 *           that alloc() will resturn the same addresses as the first time...
 */
struct spwlib_alloc_cfg {
	void *data;
	void *(*alloc)(void *data, int poolno, int hdr, int length);
	void (*reset)(void *data);
};

/* Allocate memory for Pool and packet structures, and initializes it.
 * However it does not allocate packet DATA/HEADER Buffers.
 */
extern struct spwlib_poolset *spwlib_poolset_alloc(struct spwlib_pool_cfg *cfg);

/* Reset packet structure fields, and reinitialize pool's packet list
 * NOTE: Can only be called after spwlib_poolset_pkt_alloc() has been called
 *       once.
 */
extern int spwlib_pool_reinit(struct spwlib_pool *pool);

/* Same As above but for every pool in the pools structure */
extern int spwlib_poolset_reinit(struct spwlib_poolset *poolset);

/* Setup every Pool in the Pools structure:
 *   - Pool Packet Lists filled with packet
 *   - All Packets in all Pools are initialized
 *   - Packets are assigned a DATA and HEADER pointer.
 */
extern int spwlib_poolset_pkt_alloc(
	struct spwlib_poolset *poolset,
	struct spwlib_alloc_cfg *memalgo
	);

/* Take one packet from any matching Pool in Poolset */
extern struct spwlib_pkt *spwlib_pkt_take(
	struct spwlib_poolset *poolset,
	int data_size,
	int header_size
	);

/* Return one packet to the Pool is belongs to */
extern void spwlib_pkt_return(struct spwlib_pkt *pkt);

/* Take multiple packet from any matching Pool */
extern int spwlib_pkt_chain_take(
	struct spwlib_poolset *poolset,
	int data_size,
	int header_size,
	int max,
	struct spwlib_list *list
	);

extern void spwlib_poolset_print(struct spwlib_poolset *poolset);

/*** GRSPW SpaceWire Packet List Handling Routines ***/

static inline void spwlib_list_clr(struct spwlib_list *list)
{
        list->head = NULL;
        list->tail = NULL;
	list->count = 0;
}

static inline int spwlib_list_is_empty(struct spwlib_list *list)
{
        return (list->head == NULL);
}

/* Return Number of entries in list */
static inline int spwlib_list_cnt(struct spwlib_list *list)
{
	return list->count;
}

static inline void spwlib_list_set_count(struct spwlib_list *list, int count)
{
	list->count = count;
}

/* Count number of packets regardless of count variable */
extern int spwlib_list_count(struct spwlib_list *list);

static inline void
spwlib_list_append(struct spwlib_list *list, struct spwlib_pkt *pkt)
{
	pkt->next = NULL;
	if ( list->tail == NULL ) {
		list->head = pkt;
	} else {
		list->tail->next = pkt;
	}
	list->tail = pkt;
	list->count++;
}

static inline void 
spwlib_list_prepend(struct spwlib_list *list, struct spwlib_pkt *pkt)
{
	pkt->next = list->head;
	if ( list->head == NULL ) {
		list->tail = pkt;
	}
	list->head = pkt;
	list->count++;
}

static inline void
spwlib_list_append_list(struct spwlib_list *list, struct spwlib_list *alist)
{
	alist->tail->next = NULL;
	if ( list->tail == NULL ) {
		list->head = alist->head;
	} else {
		list->tail->next = alist->head;
	}
	list->tail = alist->tail;
	list->count += alist->count;
}

static inline void
spwlib_list_prepend_list(struct spwlib_list *list, struct spwlib_list *alist)
{
	if ( list->head == NULL ) {
		list->tail = alist->tail;
	} else {
		alist->tail->next = list->head;
	}
	list->head = alist->head;
	list->count += alist->count;
}

/* Remove dlist (delete-list) from head of list */
static inline void
spwlib_list_remove_head_list(struct spwlib_list *list, struct spwlib_list *dlist)
{
	list->head = dlist->tail->next;
	if ( list->head == NULL ) {
		list->tail = NULL;
	}
	dlist->tail->next = NULL;
	list->count -= dlist->count;
}

/* Take A number of entries from head of list 'list' and put the entires
 * to rlist (result list).
 */
extern int spwlib_list_take_head_list(struct spwlib_list *list,
				struct spwlib_list *rlist, int max);

#endif
