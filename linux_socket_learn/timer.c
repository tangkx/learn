#include <time.h>
#include <timer.h>
#include <stdint.h>
#include <stdlib.h>


#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR - 1)
#define TIME_LEVEL_MASK (TIME_LEVEL - 1)

struct timer_node {
    struct timer_node *next;
    uint32_t expire;
};

struct link_list {
    struct timer_node head;
    struct timer_node *tail;
};

struct timer{
    struct link_list near[TIME_NEAR];
    struct link_list t[4][TIME_LEVEL];
    uint32_t time;
    uint32_t current;
    uint32_t starttime;
    uint64_t current_point;
    uint64_t origin_point;
};

static struct timer *TI = NULL;

static inline struct timer_node *
link_clear(struct link_list *l){
    struct timer_node *ret = l->head.next;
    l->head.next = 0;
    l->tail = &(l->head);
    return ret;
}

static inline void
link(struct link_list *l, struct timer_node *node){
    l->tail->next = node;
    l->tail = node;
    node->next = 0;
}

static void
add_node(struct timer *T, struct timer_node *node){
    uint32_t time = node->expire;
    uint32_t current = T->time;

    if((time | TIME_NEAR_MASK) == (current | TIME_NEAR_MASK)){
        link(&(T->near[time & TIME_NEAR_MASK]), node);
    }else{
        uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
        int i;
        for(i=0; i<3; i++){
            if((time | (mask-1)) == (current | (mask-1))){
                break;
            }
            mask = mask << TIME_LEVEL_SHIFT;
        }
        link(&T->t[((time >> (TIME_NEAR_SHIFT + i*TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
    }
}


