#include <slist.h>

/* Return Number of entries in slist */
int slist_cnt(struct slist *slist)
{
        struct slistnode *lastnode = NULL;
        struct slistnode *node = slist->head;
        int cnt = 0;
        while (node) {
                cnt++;
                lastnode = node;
                node = node->next;
        }
        if (lastnode && (slist->tail != lastnode)) {
                return -1;
        }

        return cnt;
}

void slist_addtail(struct slist *slist, struct slistnode *node)
{
        node->next = NULL;
        if ( slist->tail == NULL ) {
                slist->head = node;
        } else {
                slist->tail->next = node;
        }
        slist->tail = node;
}

struct slistnode *slist_remhead(struct slist *list)
{
        struct slistnode *head;

        if (slist_is_empty(list)) {
                return NULL;
        }
        head = list->head;
        list->head = head->next;
        if (list->head == NULL) {
                list->tail = NULL;
        }

        return head;
}

