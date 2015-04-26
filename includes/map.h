
#ifndef _MAP_H
#define _MAP_H

#include "class.h"
#include "binary_tree.h"

#define CLASS_NAME(a,b) a## Map ##b

struct map_bucket
{
    struct tree_node node;
    struct list_head list;
};

struct map_entry
{
    struct list_head list;
    void *key;
};

CLASS(Object,
    long long (*hash_func)(void *a),
    char (*equals_func)(void *a, void *b))

    BinaryTree btree;
    void METHOD(put, void *key, struct map_entry *value);
    struct map_entry *METHOD(get, void *key);
    struct map_entry *METHOD(remove, void *key);

    long long (*hash_func)(void *a);
    char (*equals_func)(void *a, void *b);
END_CLASS

#undef CLASS_NAME // Map

#endif // !_MAP_H
