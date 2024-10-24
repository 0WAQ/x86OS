/**
 * 
 * 链表的实现
 * 
 */
#include "tools/list.h"

void list_init(list_t* list) {
    list->first = list->last = NODE_NULL;
    list->count = 0;
}

void list_insert_first(list_t* list, list_node_t* node) {

    // 先调整node节点自身的指针
    node->next = list->first;
    node->prev = NODE_NULL;

    if(list_is_empty(list)) {
        list->first = list->last = node;
    } else {    // 若表不为空, 则设置后继
        list->first->prev = node;
        list->first = node;
    }

    ++list->count;
}

void list_insert_last(list_t* list, list_node_t* node) {
    
    // 先调整node节点自身的指针
    node->prev = list->last;
    node->next = NODE_NULL;

    if(list_is_empty(list)) {
        list->first = list->last = node;
    } else {    // 若表不为空, 则还调整前驱
        list->last->next = node;
        list->last = node;
    }

    ++list->count;
}

list_node_t* list_remove_first(list_t* list) {
    // 表项为空, 返回空
    if(list_is_empty(list)) {
        return NODE_NULL;
    }

    // 取出第一个节点(待删除节点)
    list_node_t* node = list->first;
    
    // 将first往后移一个, 若node没有后继, 则first为空
    list->first = node->next;
    if(list->first == NODE_NULL) {
        list->last = list->first;
    }
    else {    // 若first不为空, 即node有后继, 调整后继
        node->next->prev = NODE_NULL;
    }

    // 清空node
    node->next = node->prev = NODE_NULL;
    --list->count;
    
    return node;
}

list_node_t* list_remove_last(list_t* list) {
    // 表项为空, 返回空
    if(list_is_empty(list)) {
        return NODE_NULL;
    }

    // 取出最后一个节点(待删除节点)
    list_node_t* node = list->last;

    // 将last往前移一个, 若node没有前驱, 则last为空
    list->last = node->prev;
    if(list->last == NODE_NULL) {
        list->first = list->last;
    }
    else {  // 若last不为空, 即node有前驱, 则调整前驱
        node->prev->next = NODE_NULL;
    }

    // 清空node
    node->next = node->prev = NODE_NULL;
    --list->count;

    return node;
}

list_node_t* list_remove(list_t* list, list_node_t* node) {
    
    // 如果是first, first往后移
    if(node == list->first) {
        list->first = node->next;
    }
    
    // 如果是last, 则last往前移
    if(node == list->last) {
        list->last = node->prev;
    }

    // 如果有前, 则调整后继
    if(node->prev) {
        node->prev->next = node->next;
    }

    // 如果有后, 则调整前驱
    if(node->next) {
        node->next->prev = node->prev;
    }

    // 清空node
    node->prev = node->next = NODE_NULL;
    --list->count;

    return node;
}