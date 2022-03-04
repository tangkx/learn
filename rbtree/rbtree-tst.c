
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "rbtree.h"

struct mynode
{
    struct rb_node node;
    char *string;
};

struct rb_root mytree = RB_ROOT;

struct mynode *my_search(struct rb_root *root, char *string)
{
    struct rb_node *node = root->rb_node;

    while(node)
    {
        struct mynode *data = container_of(node, struct mynode, node);
        int result;

        result = strcmp(string, data->string);

        if(result < 0)
            node = node->rb_left;
        else if(result > 0)
            node = node->rb_right;
        else
            return data;
    }

    return NULL;
}

int my_insert(struct rb_root *root, struct mynode *data)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    while(*new)
    {
        struct mynode *this = container_of(*new, struct mynode, node);
        int result = strcmp(data->string, this->string);

        parent = *new;
        if(result < 0)
            new = &((*new)->rb_left);
        else if(result > 0)
            new = &((*new)->rb_right);
        else 
            return 0;
    }

    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, root);

    return 1;
}

void my_free(struct mynode *node)
{
    if(node != NULL)
    {
        if(node->string != NULL)
        {
            free(node->string);
            node->string = NULL;
        }
        free(node);
        node = NULL;
    }
}

#define NUM_NODES 100000

int main()
{
    struct mynode *mn[NUM_NODES];

    int i = 0;
    printf("insert node from 1 to NUM_NODES(%d)\n",NUM_NODES);
    for(; i < NUM_NODES; i++)
    {
        mn[i] = (struct mynode *)malloc(sizeof(struct mynode));
        mn[i]->string = (char *)malloc(sizeof(char) * 4);
        sprintf(mn[i]->string, "%d", i);
        my_insert(&mytree, mn[i]);
    }

    struct rb_node *node;
    printf("search all nodes: \n");
    for(node = rb_first(&mytree); node; node = rb_next(node))
        printf("key = %s\n", rb_entry(node, struct mynode, node)->string);
    
    printf("delete node 20: \n");
    struct mynode *data = my_search(&mytree, "20");
    if(data)
    {
        rb_erase(&data->node, &mytree);
        my_free(data);
    }

    printf("delete node 15: \n");
	data = my_search(&mytree, "15");
	if (data) {
		rb_erase(&data->node, &mytree);
		my_free(data);
	}

   printf("search again:\n");
	for (node = rb_first(&mytree); node; node = rb_next(node))
		printf("key = %s\n", rb_entry(node, struct mynode, node)->string);
	return 0;
}