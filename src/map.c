
#include <string.h>

#include "map.h"

#define CLASS_NAME(a,b) a## Map ##b
Map METHOD_IMPL(construct, 
    long long (*hash_func)(void *a),
    char (*equals_func)(void *a, void *b))
{
    this->btree = NEW(BinaryTree);
    this->hash_func = hash_func;
    this->equals_func = equals_func;
    return this;
}

void free_bucket(void *ctxt, struct tree_node *node)
{
    free(list_entry(node, struct map_bucket, node));
}

void METHOD_IMPL(deconstruct)
{
    CALL(this->btree, iterate, free_bucket, NULL);
    DELETE(this->btree);
}

void METHOD_IMPL(put, void *key, struct map_entry *value)
{
    value->key = key;
    long long hash = this->hash_func(key);

    struct tree_node *_n = CALL(this->btree, get, hash);
    struct map_bucket *n;

    if(!_n)
    {
        n = (struct map_bucket*)malloc(sizeof(struct map_bucket));
        INIT_LIST_HEAD(&n->list);
        CALL(this->btree, put, hash, &n->node);
    }
    else
    {
        n = list_entry(_n, struct map_bucket, list);
    }

    list_add_tail(&value->list, &n->list);
}

struct map_entry *METHOD_IMPL(get_from_hash, long long hash, void *key, struct map_bucket **bucket)
{
    struct tree_node *node = CALL(this->btree, get, hash);
    if(!node)
        return NULL;
    struct map_bucket *entry =
        list_entry(node, struct map_bucket, node);
    if(bucket)
        *bucket = entry;

    struct map_entry *i;
    list_for_each_entry(i, &entry->list, list)
    {
        if(this->equals_func(key, i->key))
            return i;
    }
    return NULL;
}

struct map_entry *METHOD_IMPL(get, void *key)
{
    long long hash = this->hash_func(key);
    return PRIV_CALL(this, get_from_hash, hash, key, NULL);
}

struct map_entry *METHOD_IMPL(remove, void *key)
{
    long long hash = this->hash_func(key);
    struct map_bucket *bucket;
    struct map_entry *entry = PRIV_CALL(this, get_from_hash, hash, key, &bucket);

    if(!entry)
        return NULL;

    list_del(&entry->list);
    /* only entry left in this bucket */
    if(list_empty(&bucket->list))
    {
        CALL(this->btree, remove, hash);
        free(bucket);
    }

    return entry;
}

VIRTUAL(Object)
    VMETHOD_BASE(Object, construct);
    VMETHOD_BASE(Object, deconstruct);
    VMETHOD(put);
    VMETHOD(get);
    VMETHOD(remove);

    VFIELD(hash_func) = NULL;
    VFIELD(equals_func) = NULL;
END_VIRTUAL
#undef CLASS_NAME // Map
