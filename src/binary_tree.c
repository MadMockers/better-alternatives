
#include <string.h>

#include "binary_tree.h"

// TODO: replace with a self balancing binary search tree
// (such as a red-black tree)

#define CLASS_NAME(a,b) a## BinaryTree ##b

static inline void force_link(struct tree_node *h, struct tree_node *prev)
{
    if(h->parent)
    {
        if(h->parent->children[0] == prev)
            h->parent->children[0] = h;
        if(h->parent->children[1] == prev)
            h->parent->children[1] = h;
    }
    if(h->children[0])
        h->children[0]->parent = h;
    if(h->children[1])
        h->children[1]->parent = h;
}

static void METHOD_IMPL(priv_remove, struct tree_node *node)
{
    struct tree_node *parent = node->parent;
    struct tree_node *replace = node->children[0];
    struct tree_node *add = NULL;
    if(replace)
        add = node->children[1];
    else
        replace = node->children[1];

    if(replace)
    {
        replace->parent = parent;
        force_link(replace, node);
    }

    if(add)
        CALL(this, put, add->priority, add);

}

static void METHOD_IMPL(priv_iterate, struct tree_node *n, kv_iterator_t it, void *ctxt)
{
    if(!n)
        return;
    PRIV_CALL(this, priv_iterate, n->children[0], it, ctxt);
    PRIV_CALL(this, priv_iterate, n->children[1], it, ctxt);
    if(it(ctxt, &n->priority, n->ctxt) == ITR_REMOVE)
    {
        PRIV_CALL(this, priv_remove, n);
        free(n);
    }
}

void METHOD_IMPL(iterate, kv_iterator_t it, void *ctxt)
{
    PRIV_CALL(this, priv_iterate, this->root, it, ctxt);
}

char METHOD_IMPL(put, long long key, void *value)
{
    struct tree_node *node = malloc(sizeof(*node));
    memset(node, '\0', sizeof(*node));
    node->ctxt = value;
    node->priority = key;
    if(!this->root)
    {
        this->root = node;
        return 1;
    }

    struct tree_node *trav = this->root;
    while(1)
    {
        if(key < trav->priority)
        {
            if(trav->children[0])
            {
                trav = trav->children[0];
                continue;
            }
            trav->children[0] = node;
            return 1;
        }
        else if(key > trav->priority)
        {
            if(trav->children[1])
            {
                trav = trav->children[1];
                continue;
            }
            trav->children[1] = node;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void *METHOD_IMPL(get, long long key)
{
    if(!this->root)
        return NULL;

    struct tree_node *trav = this->root;

    while(trav)
    {
        if(key < trav->priority)
        {
            trav = trav->children[0];
        }
        else if(key > trav->priority)
        {
            trav = trav->children[1];
        }
        else
        {
            return trav->ctxt;
        }
    }
    return NULL;
}

void *METHOD_IMPL(remove, long long key)
{
    struct tree_node *node = CALL(this, get, key);
    if(!node)
        return NULL;

    PRIV_CALL(this, priv_remove, node);
    void *ret = node->ctxt;
    free(node);
    return ret;
}

VIRTUAL(Object)
    VMETHOD(put);
    VMETHOD(get);
    VMETHOD(remove);
    VMETHOD(iterate);

    VFIELD(root) = NULL;
END_VIRTUAL

#undef CLASS_NAME // BinaryTree
