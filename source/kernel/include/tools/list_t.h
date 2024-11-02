/**
 * 
 * 链表操作头文件
 * 
 */
#ifndef LIST_T_H
#define LIST_T_H

#include "common/types.h"

/**
 * @brief 链表节点
 */
typedef struct _list_node_t {
    struct _list_node_t* prev;
    struct _list_node_t* next;
}list_node_t;

/**
 * @brief 链表
 */
typedef struct _list_t {
    list_node_t* first;
    list_node_t* last;
    uint32_t count;
}list_t;

#endif // LIST_T_H