
#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include "class.h"
#include "list.h"
#include "tree.h"
#include "iterator.h"

#define CLASS_NAME(a,b) a## BinaryTree ##b
CLASS(Object)
    char METHOD(put, long long key, void *value);
    void *METHOD(get, long long key);
    void *METHOD(remove, long long key);
    void METHOD(iterate, kv_iterator_t, void *ctxt);

    struct tree_node *root;
END_CLASS
#undef CLASS_NAME // BinaryTree

#endif // !BINARY_TREE_H
