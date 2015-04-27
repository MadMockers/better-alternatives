
#ifndef ITERATOR_H
#define ITERATOR_H

typedef enum
{
    ITR_OK  = 0,
    ITR_REMOVE,
} itr_ret_t;

typedef itr_ret_t(*iterator_t)(void *ctxt, void *item);
typedef itr_ret_t(*kv_iterator_t)(void *ctxt, void *key, void *value);

#endif
