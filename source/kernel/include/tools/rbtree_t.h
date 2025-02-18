/**
 * 
 * rbtree基本类型
 * 
 */
#ifndef RBTREE_T_H
#define RBTREE_T_H

#include "common/types.h"
#include "common/macros.h"

/***
 * 
 * @brief 红黑树的性质
 * 
 * 1. 每个节点都会有颜色, 要么是红色, 要么是黑色 
 * 2. 若一个节点为红色, 则它的两个子节点为黑色 (即没有相邻的红色节点)
 * 3. 所有叶子节点(NULL节点)都是黑色的
 * 4. 以任意节点为树根时, 从树根到该树任意子节点的任意路径之间具有相同数量的黑色节点(黑色高度相同)
 * 5. 根节点是黑色的
 * 
 */

// 节点颜色 
#define RB_RED          0
#define RB_BLACK        1

/**
 * @brief rbtree节点, 通过container_of取节点元素
 */
typedef struct _rb_node {
    struct _rb_node* rb_parent;
    int rb_color;
    struct _rb_node* rb_right;
    struct _rb_node* rb_left;
}rb_node;

/**
 * @brief 红黑树根节点
 */
typedef struct _rb_root {
    struct _rb_node* rb_node;
}rb_root;


#define RB_ROOT             ((rb_root){NULL, })
#define rb_parent(r)        ((rb_node*)((r)->rb_parent))
#define rb_color(r)         ((r)->rb_color)
#define rb_is_red(r)        (!rb_color(r))
#define rb_is_black(r)      (rb_color(r))
#define rb_set_red(r)       do { (r)->rb_color = RB_RED;   } while(0)
#define rb_set_black(r)     do { (r)->rb_color = RB_BLACK; } while(0)
#define rb_set_color(l, r)  do { (l)->rb_color = (r)->rb_color; } while(0)


#endif // RBTREE_T_H