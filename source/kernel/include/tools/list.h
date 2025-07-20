#ifndef LIST_H
#define LIST_H

#include "common/types.h"
#include "common/macros.h"
#include "tools/list_t.h"

#ifdef entry_of
#endif

void list_init(list_t* list);
void list_insert_first(list_t* list, list_node_t* node);
void list_insert_last(list_t* list, list_node_t* node);
list_node_t* list_remove_first(list_t* list);
list_node_t* list_remove_last(list_t* list);
list_node_t* list_remove(list_t* list, list_node_t* node);

static inline void list_node_init(list_node_t* node) { node->prev = node->next = NULL; }
static inline list_node_t* list_node_prev(list_node_t* node) { return node->prev; }
static inline list_node_t* list_node_next(list_node_t* node) { return node->next; }
static inline bool list_is_empty(list_t* list) { return list->count == 0; }
static inline u32_t list_count(list_t* list) { return list->count; }
static inline list_node_t* list_first(list_t* list) { return list->first; }
static inline list_node_t* list_last(list_t* list) { return list->last; }

#endif // LIST_H