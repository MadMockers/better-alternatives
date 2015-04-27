
#include <string.h>

#include "map.h"

struct map_bucket
{
    struct list_head list;
};

struct map_entry
{
    struct list_head list;
    void *key;
    void *obj;
};

/* generic hash / equals funcs */
long long str_hash(void *a)
{
    unsigned char *str = a;
    long long hash = 5381;
    int c;

    while ( (c = *str++) )
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

char str_equals(void *a, void *b)
{
    return !strcmp(a, b);
}

long long int_hash(void *a)
{
    return *(int*)a;
}

char int_equals(void *a, void *b)
{
    return *(int*)a == *(int*)b;
}

#define CLASS_NAME(a,b) a## Map ##b
static Map METHOD_IMPL(construct, 
    long long (*hash_func)(void *a),
    char (*equals_func)(void *a, void *b))
{
    this->btree = NEW(BinaryTree);
    this->hash_func = hash_func;
    this->equals_func = equals_func;
    return this;
}

static itr_ret_t remove_all(void *ctxt, void *key, void *obj)
{
    return ITR_REMOVE;
}

static void METHOD_IMPL(deconstruct)
{
    CALL(this, iterate, remove_all, NULL);
    DELETE(this->btree);
}

static void METHOD_IMPL(put, void *key, void *obj)
{
    struct map_entry *entry = malloc(sizeof(*entry));
    entry->key = key;
    entry->obj = obj;
    long long hash = this->hash_func(key);

    struct map_bucket *n = CALL(this->btree, get, hash);

    if(!n)
    {
        n = (struct map_bucket*)malloc(sizeof(struct map_bucket));
        INIT_LIST_HEAD(&n->list);
        CALL(this->btree, put, hash, n);
    }

    list_add_tail(&entry->list, &n->list);
}

static struct map_entry *METHOD_IMPL(get_from_hash, long long hash, void *key, struct map_bucket **bucket)
{
    struct map_bucket *entry = CALL(this->btree, get, hash);
    if(!entry)
        return NULL;
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

static void *METHOD_IMPL(get, void *key)
{
    long long hash = this->hash_func(key);
    struct map_entry *entry = PRIV_CALL(this, get_from_hash, hash, key, NULL);
    if(!entry)
        return NULL;
    return entry->obj;
}

static struct map_entry *METHOD_IMPL(remove, void *key)
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

    void *ret = entry->obj;
    free(entry);
    return ret;
}

struct itr_ctxt
{
    kv_iterator_t itr;
    void *usr_ctxt;
};

static itr_ret_t btree_iterator(struct itr_ctxt *ctxt, long long *key, struct map_bucket *b)
{
    struct map_entry *i, *j;
    list_for_each_entry_safe(i, j, &b->list, list)
    {
        itr_ret_t ret = ctxt->itr(ctxt->usr_ctxt, i->key, i->obj);
        if(ret == ITR_REMOVE)
        {
            list_del(&i->list);
            free(i);
        }
    }
    if(list_empty(&b->list))
    {
        free(b);
        return ITR_REMOVE;
    }
    return ITR_OK;
}

static void METHOD_IMPL(iterate, kv_iterator_t it, void *usr_ctxt)
{
    struct itr_ctxt ctxt = {
        .itr = it,
        .usr_ctxt = usr_ctxt,
    };
    CALL(this->btree, iterate, (kv_iterator_t)btree_iterator, &ctxt);
}

static itr_ret_t to_list_iterator(List l, void *key, void *obj)
{
    CALL(l, append, obj);
    return ITR_OK;
}

static List METHOD_IMPL(to_list)
{
    List l = NEW(List);
    CALL(this, iterate, (kv_iterator_t)to_list_iterator, l);
    return l;
}

VIRTUAL(Object)
    VMETHOD_BASE(Object, construct);
    VMETHOD_BASE(Object, deconstruct);
    VMETHOD(put);
    VMETHOD(get);
    VMETHOD(remove);
    VMETHOD(iterate);
    VMETHOD(to_list);

    VFIELD(hash_func) = NULL;
    VFIELD(equals_func) = NULL;
END_VIRTUAL
#undef CLASS_NAME // Map
