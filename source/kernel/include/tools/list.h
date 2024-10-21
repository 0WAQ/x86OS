/**
 * 
 * 链表基本类型及相关参数
 * 
 */
#ifndef LIST_H
#define LIST_H

#include "list_t.h"
#include "common/macros.h"

/**
 * 封装container_of
 */
#define list_entry_of(member, container_type) \
    entry_of(member, container_type, list_node)

/**
 * @brief 初始化链表
 */
void list_init(list_t* list);

/**
 * @brief 头插
 */
void list_insert_first(list_t* list, list_node_t* node);

/**
 * @brief 尾插
 */
void list_insert_last(list_t* list, list_node_t* node);


/**
 * @brief 头删
 */
list_node_t* list_remove_first(list_t* list);

/**
 * @brief 尾删
 */
list_node_t* list_remove_last(list_t* list);

/**
 * @brief 任意位置删除
 */
list_node_t* list_insert(list_t* list, list_node_t* node);


static inline 
void list_node_init(list_node_t* node) {
    node->prev = node->next = NODE_NULL;
}

static inline 
list_node_t* list_node_prev(list_node_t* node) {
    return node->prev;
}

static inline 
list_node_t* list_node_next(list_node_t* node) {
    return node->next;
}

static inline
int list_is_empty(list_t* list) {
    return list->count == 0;
}

static inline
uint32_t list_count(list_t* list) {
    return list->count;
}

static inline
list_node_t* list_first(list_t* list) {
    return list->first;
}

static inline
list_node_t* list_last(list_t* list) {
    return list->last;
}

#endif // LIST_H