#ifndef _SLIST_H_
#define _SLIST_H_

/* Single linked list interface */

#include <stddef.h>

struct slistnode {
        struct slistnode *next;
};

struct slist {
        struct slistnode *head;
        struct slistnode *tail;
};

static inline void slist_init(struct slist *slist)
{
        slist->head = NULL;
        slist->tail = NULL;
}

static inline int slist_is_empty(struct slist *slist)
{
        return slist->head == NULL;
}

/* Return Number of entries in slist */
int slist_cnt(struct slist *slist);

/* Add a node to the tail of the list */
void slist_addtail(struct slist *slist, struct slistnode *node);

/* Removes and returns the node at the head */
struct slistnode *slist_remhead(struct slist *list);

#endif

