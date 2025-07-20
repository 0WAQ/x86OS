#ifndef LIST_T_H
#define LIST_T_H

#include "common/types.h"

typedef struct _list_node_t {
    struct _list_node_t* prev;
    struct _list_node_t* next;
}list_node_t;

typedef struct _list_t {
    list_node_t* first;
    list_node_t* last;
    u32_t count;
}list_t;

#endif // LIST_T_H