/**
 * 
 * 链表基本类型及相关参数
 * 
 */
#ifndef LIST_H
#define LIST_H

#include "list_t.h"

static inline 
void list_node_init(list_node_t* node) {
    node->prev = node->next = (list_node_t*)0;
}

static inline 
list_node_t* list_node_prev(list_node_t* node) {
    return node->prev;
}

static inline 
list_node_t* list_node_next(list_node_t* node) {
    return node->next;
}

/**
 * @brief 初始化链表
 */
void list_init(list_t* list);


#endif // LIST_H