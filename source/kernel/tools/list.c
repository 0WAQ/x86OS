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
    node->next = list->first;
    node->prev = NODE_NULL;

    if(list_is_empty(list)) {
        list->first = list->last = node;
    } else {
        list->first->prev = node;
        list->first = node;
    }

    ++list->count;
}

void list_insert_last(list_t* list, list_node_t* node) {
    node->prev = list->last;
    node->next = NODE_NULL;

    if(list_is_empty(list)) {
        list->first = list->last = node;
    } else {
        list->last->next = node;
        list->last = node;
    }

    ++list->count;
}

list_node_t* list_remove_first(list_t* list) {
    if(list_is_empty(list)) {
        return NODE_NULL;
    }

    list_node_t* remove_node = list->first;
    list->first = remove_node->next;
    if(list->first == NODE_NULL) {
        list->last = list->first;
    } else {
        remove_node->next->prev = NODE_NULL;
    }
    remove_node->next = remove_node->prev = NODE_NULL;
    --list->count;
    
    return remove_node;
}

list_node_t* list_remove_last(list_t* list) {
    // TODO:
}

list_node_t* list_insert(list_t* list, list_node_t* node) {
    if(list->first = node) {
        list->first = node->next;
    }
    
    if(list->last = node) {
        list->last = node->prev;
    }

    if(node->prev) {
        node->prev->next = node->next;
    }

    if(node->next) {
        node->next->prev = node->prev;
    }

    node->prev = node->next = NODE_NULL;
    --list->count;

    return node;
}