
#include "oolist.h"

#define CLASS_NAME(a,b) a## List ##b

struct list_container
{
    struct list_head list;
    void *item;
};

static List METHOD_IMPL(construct)
{
    CONSTRUCT_CALL();
    INIT_LIST_HEAD(&this->list);
    return this;
}

static void METHOD_IMPL(deconstruct)
{
    struct list_container *i, *j;
    list_for_each_entry_safe(i, j, &this->list, list)
    {
        free(i);
    }
    SUPER_CALL(Object, this, deconstruct);
}

static struct list_head *METHOD_IMPL(get_list_at_index, size_t index)
{
    if(index > this->size || index < 0 || this->size == 0)
        return NULL;

    struct list_head *ret = &this->list;
    if(index < this->size - index)
    {
        size_t count = index + 1, i;
        for(i = 0;i < count;i++)
            ret = ret->next;
    }
    else
    {
        size_t count = this->size - index, i;
        for(i = 0;i < count;i++)
            ret = ret->prev;
    }
    return ret;
}

static int METHOD_IMPL(add, void *item, size_t index)
{
    struct list_head *insert_at = 
        this->size?
            PRIV_CALL(this, get_list_at_index, index):
            &this->list;
    if(!insert_at)
        return -1;

    struct list_container *container = malloc(sizeof(*container));
    container->item = item;

    list_add_tail(&container->list, insert_at);

    this->size++;
    return 0;
}

static int METHOD_IMPL(push, void *item)
{
    return CALL(this, add, item, 0);
}

static int METHOD_IMPL(append, void *item)
{
    return CALL(this, add, item, this->size);
}

static void *METHOD_IMPL(get, size_t index)
{
    struct list_head *get_at = PRIV_CALL(this, get_list_at_index, index);
    if(!get_at)
        return NULL;

    struct list_container *cont = list_entry(get_at, typeof(*cont), list);
    return cont->item;
}

static void *METHOD_IMPL(peek)
{
    return CALL(this, get, 0);
}

static void *METHOD_IMPL(remove, size_t index)
{
    struct list_head *remove_at = PRIV_CALL(this, get_list_at_index, index);
    if(!remove_at)
        return NULL;

    list_del(remove_at);
    struct list_container *cont;
    void *ret;
    cont = list_entry(remove_at, struct list_container, list);
    ret = cont->item;
    free(cont);
    this->size--;
    return ret;
}

static void *METHOD_IMPL(pop)
{
    return CALL(this, remove, 0);
}

static void METHOD_IMPL(iterate, int(*it)(void *ctxt, void *item), void *ctxt)
{
    struct list_container *i, *j;
    list_for_each_entry_safe(i, j, &this->list, list)
    {
        if(it(ctxt, i->item) == ITR_REMOVE)
        {
            list_del(&i->list);
            free(i);
        }
    }
}

VIRTUAL(Object)
    VMETHOD_BASE(Object, construct);
    VMETHOD_BASE(Object, deconstruct);

    VMETHOD(add);
    VMETHOD(push);
    VMETHOD(append);

    VMETHOD(get);
    VMETHOD(peek);

    VMETHOD(remove);
    VMETHOD(pop);

    VMETHOD(iterate);

    VFIELD(size) = 0;
END_VIRTUAL

#undef CLASS_NAME // List
