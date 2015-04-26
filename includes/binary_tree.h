
#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include "list.h"
#include "tree.h"
#include "class.h"

#define CLASS_NAME(a,b) a## BinaryTree ##b
CLASS(Object)
    char METHOD(put, long long key, struct tree_node *node);
    struct tree_node *METHOD(get, long long key);
    struct tree_node *METHOD(remove, long long key);
    void METHOD(iterate, void (*iterator)(void *ctxt, struct tree_node*), void *ctxt);

    struct tree_node *root;
END_CLASS
#undef CLASS_NAME // BinaryTree

#endif // !BINARY_TREE_H
