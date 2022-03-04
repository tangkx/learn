#ifndef __LIST_H
#define __LIST_H

#include "macro.h"

struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};


#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member)  \
    t_container_of(ptr, type, member)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *in, 
                                struct list_head *prev,
                                struct list_head *next)
{
    next->prev = in;
    in->next = next;
    in->prev = prev;
    prev->next = in;
}

static inline void list_add(struct list_head *in, struct list_head *head)
{
    __list_add(in, head, head->next);
}

static inline void list_add_tail(struct list_head *in, struct list_head *head)
{
    __list_add(in, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
    __list_del_entry(entry);
    entry->prev = NULL;
    entry->next = NULL;
}

#define list_entry_is_head(pos, head, member)   \
    (&pos->member == (head))

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member)  \
    list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member)        \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head) \
    for(pos= (head)->next; pos != (head); pos = pos->next)

#define list_for_each_entry(pos, head, member)  \
    for(pos = list_first_entry(head, typeof(*pos), member);  \
        !list_entry_is_head(pos, head, member);               \
        pos = list_next_entry(pos, member))

#endif