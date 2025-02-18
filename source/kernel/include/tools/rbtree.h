/**
 * 
 * rbtree头文件
 * 
 */
#ifndef RBTREE_H
#define RBTREE_H

#include "rbtree_t.h"

/**
 * @brief 初始化新节点
 * @param rb_link 指向node要插入的位置, *rb_link是parent的子节点, 由调用者决定其是左还是右
 */
void rb_link_node(rb_node* node, rb_node* parent, rb_node** rb_link);

/**
 * @brief 左旋
 */
void __rb_rotate_left(rb_node* node, rb_root* root);

/**
 * @brief 右旋
 */
void __rb_rotate_right(rb_node* node, rb_root* root);

/**
 * @brief 修复插入节点后红黑树的不平衡问题
 * @attention 插入操作由用户定义
 * @param node 关注节点
 */
void rb_insert_color(rb_node* node, rb_root* root);

/**
 * @brief 删除节点
 */
void rb_erase(rb_node* node, rb_root* root);

/**
 * @brief 修复删除节点后红黑树的不平衡问题
 */
void __rb_erase_color(rb_node* node, rb_node* parent, rb_root* root);


rb_node* rb_next(rb_node* node);
rb_node* rb_prev(rb_node* node);
rb_node* rb_first(rb_root* root);

#define rb_entry(member_val, container_type, member_name) \
    entry_of(member_val, container_type, member_name)

#endif // RBTREE_H