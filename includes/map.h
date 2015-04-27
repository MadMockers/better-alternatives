
#ifndef _MAP_H
#define _MAP_H

#include "class.h"
#include "binary_tree.h"
#include "oolist.h"
#include "iterator.h"

#define CLASS_NAME(a,b) a## Map ##b

/* some generic hash / equals functions */
long long str_hash(void *a);
char str_equals(void *a, void *b);
long long int_hash(void *a);
char int_equals(void *a, void *b);

CLASS(Object,
    long long (*hash_func)(void *a),
    char (*equals_func)(void *a, void *b))

    BinaryTree btree;
    void METHOD(put, void *key, void *value);
    void *METHOD(get, void *key);
    void *METHOD(remove, void *key);

    void METHOD(iterate, kv_iterator_t, void *ctxt);
    List METHOD(to_list);

    long long (*hash_func)(void *a);
    char (*equals_func)(void *a, void *b);
END_CLASS

#undef CLASS_NAME // Map

#endif // !_MAP_H
