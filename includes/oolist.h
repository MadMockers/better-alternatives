
#ifndef OOLIST_H
#define OOLIST_H

#include "class.h"
#include "binary_tree.h"
#include "list.h"
#include "iterator.h"

#define CLASS_NAME(a,b) a## List ##b

CLASS(Object)
    int METHOD(add, void *item, size_t index);
    int METHOD(push, void *item);
    int METHOD(append, void *item);

    void *METHOD(get, size_t index);
    void *METHOD(peek);

    void *METHOD(remove, size_t index);
    void *METHOD(pop);

    void METHOD(iterate, iterator_t, void *ctxt);

    size_t size;
    struct list_head list;
END_CLASS

#undef CLASS_NAME // List

#endif // !OOLIST_H
